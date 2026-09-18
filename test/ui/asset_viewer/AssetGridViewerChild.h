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
		GameLoopPassedSinceRequest = 1 << 4,
		JustDragDroppedIntoGrid = 1 << 5
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
			state = 0;
		}
	};

	struct SpriteAssetGridSelection : GridSelection
	{
		std::string spriteSeries;
		Sprite sprite;

		void Reset()
		{
			spriteSeries.clear();
			sprite = {};
			ClearRename();
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

		void Reset()
		{
			audioHandle = {};
			ClearRename();
		}

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

	struct FontAssetGridSelection : GridSelection
	{
		Handle<TextureResource> fontHandle;
		std::unordered_map<Handle<TextureResource>, ImFont*> handleToGuiFont;

		void Reset()
		{
			fontHandle = {};
			ClearRename();
		}

		bool HasSelection() const
		{
			if (!fontHandle.IsValid())
			{
				return false;
			}

			auto it = handleToGuiFont.find(fontHandle);

			return (it != handleToGuiFont.end() && it->second != nullptr);
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
	const FontAssetGridSelection GetFontSelection() const { return fontSelection_; }

	AssetItem::Type GetOpenAssetTabType() const noexcept { return currentAssetTab_; }

private:
	enum ResolveAssetDragDropOutcome : uint8_t
	{
		Completed = 1 << 0,
		LoadSuccessful = 1 << 1
	};

	void DrawSpriteAssetGrid(TextureRepository& loadTargetRepo, ResourceContext& ctx);
	void DrawAudioAssetGrid(AudioBank& audioBank, ResourceContext& ctx);
	void DrawFontAssetGrid(FontAtlas& fontAtlas, ResourceContext& ctx);

	AssetItem::Type HandleDirectoryAssetDragDropTarget(const AssetItem& item, const AssetTree& assetTree,
													   SceneFixture& fixture);
	AssetItem::Type HandleSpriteAssetDragDropTarget(const AssetItem& item, SpriteAtlas& loadTargetAtlas,
													SDL_Renderer* renderer);
	AssetItem::Type HandleAudioAssetDragDropTarget(const AssetItem& item, AudioBank& audioBank);
	uint8_t ResolveAudioAssetDragDropTarget(AudioBank& audioBank);
	AssetItem::Type HandleFontAssetDragDropTarget(const AssetItem& item, FontAtlas& fontAtlas, 
												  SDL_Renderer* renderer);

	void HandleSpriteGridCellDragDropSource(const SpriteAtlas& loadTargetAtlas);
	bool HandleSpriteGridCellDragDropTarget(SpriteAtlas& loadTargetAtlas, std::string_view seriesName);

	AssetItem::Type ResolvePendingAssetDragDropTarget(SceneFixture& fixture, AssetItem::Type lastLoadedType);

	void DrawSpritePopupContextMenu(SpriteAtlas& loadTargetAtlas);
	void DrawAudioPopupContextMenu(AudioBank& audioBank);
	void DrawFontPopupContextMenu(FontAtlas& fontAtlas);

	void DrawSpriteTabItemMenu(SpriteAtlas& loadTargetAtlas);

	void ResolveSpritePopupContextActions(SpriteAtlas& loadTargetAtlas);
	void ResolveAudioPopupContextActions(AudioBank& audioBank);
	void ResolveFontPopupContextActions(FontAtlas& fontAtlas);

	void HandleSpriteSelectionRename(const AssetGridCell& gridCell, SpriteAtlas& loadTargetAtlas,
									 bool renameStartedThisFrame);
	void HandleAudioSelectionRename(const AssetGridCell& gridCell, AudioBank& audioBank,
									 bool renameStartedThisFrame);
	void HandleFontSelectionRename(const AssetGridCell& gridCell, FontAtlas& fontAtlas,
								   bool renameStartedThisFrame);

	std::optional<AssetItem::Type> forceAssetTabOpen_;
	SpriteAssetGridSelection spriteSelection_;
	AudioAssetGridSelection audioSelection_;
	FontAssetGridSelection fontSelection_;
	AssetItem::Type currentAssetTab_ = AssetItem::Type::Image;
	std::optional<AssetItem> pendingDragDropTargetItem_;
};

} // ui

#endif