#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "AssetGridCell.h"
#include "AssetTree.h"

class SceneFixture;
class AudioBank;

namespace ui {

class AssetGridViewerChild
{
public:
	enum GridSelectionState : uint8_t
	{
		Renaming = 1 << 0,
		RequestErase = 1 << 1,
		ViewingInsideSeries = 1 << 2,
		RequestConvertAudioType = 1 << 3,
		GameLoopPassedSinceRequest = 1 << 4
	};

	struct GridSelection
	{
		std::string currentRename;
		uint8_t state = 0;

		bool IsRenaming() const
		{
			return (state & GridSelectionState::Renaming) != 0;
		}

		void ClearRename()
		{
			currentRename.clear();
			state &= ~GridSelectionState::Renaming;
		}
	};

	struct SpriteAssetGridSelection : GridSelection
	{
		std::string spriteSeries;
		Sprite sprite;

		void Reset()
		{
			spriteSeries.clear();
			currentRename.clear();
			sprite = {};
			state &= ~GridSelectionState::Renaming;
		}

		bool SpriteSeriesSelectedAtTopLevel() const
		{
			return !spriteSeries.empty() && !sprite.resourceHandle.IsValid() && 
				((state & GridSelectionState::ViewingInsideSeries) == 0);
		}

		bool HasSelection() const
		{
			return (!spriteSeries.empty() || sprite.resourceHandle.IsValid());
		}
	};

	struct AudioAssetGridSelection : GridSelection
	{
		Handle<Audio> audioHandle;
		//Entity_t playerEntityId = kInvalidEntity;

		bool HasSelection() const
		{
			return audioHandle.IsValid();
		}

		void UpdateGameLoopPassedFlag()
		{
			if (state & (GridSelectionState::RequestConvertAudioType |
						 GridSelectionState::RequestErase))
			{
				state |= GridSelectionState::GameLoopPassedSinceRequest;
			}
		}
	};

	struct ResourceContext
	{
		const AssetTree& assetTree;
		const AssetViewerIcons& icons;
		const GuiTextureConverter& uiTexturesConverter;
	};

	void Draw(SceneFixture& fixture, ResourceContext& ctx);

	void HandleAssetDragDropTarget(SceneFixture& fixture, ResourceContext& ctx);

	const SpriteAssetGridSelection GetSpriteSelection() const { return spriteSelection_; }
	const AudioAssetGridSelection GetAudioSelection() const { return audioSelection_; }

private:
	void DrawSpriteAssetGrid(TextureRepository& loadTargetRepo, ResourceContext& ctx);
	void DrawAudioAssetGrid(AudioBank& audioBank, ResourceContext& ctx);

	AssetItem::Type HandleDirectoryAssetDragDropTarget(const AssetItem& item, const AssetTree& assetTree,
													   SceneFixture& fixture);
	AssetItem::Type HandleSpriteAssetDragDropTarget(const AssetItem& item, SpriteAtlas& loadTargetAtlas,
													SDL_Renderer* renderer);
	AssetItem::Type HandleAudioAssetDragDropTarget(const AssetItem& item, AudioBank& audioBank);

	void DrawSpritePopupContextMenu(SpriteAtlas& loadTargetAtlas);
	void DrawAudioPopupContextMenu(AudioBank& audioBank);

	void ResolveSpritePopupContextActions(SpriteAtlas& loadTargetAtlas);
	void ResolveAudioPopupContextActions(AudioBank& audioBank);

	void HandleSpriteSelectionRename(const AssetGridCell& gridCell, SpriteAtlas& loadTargetAtlas,
									 bool renameStartedThisFrame);
	void HandleAudioSelectionRename(const AssetGridCell& gridCell, AudioBank& audioBank,
									 bool renameStartedThisFrame);

	std::optional<AssetItem::Type> forceAssetTabOpen_;
	SpriteAssetGridSelection spriteSelection_;
	AudioAssetGridSelection audioSelection_;
};

} // ui

#endif