#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "AssetGridCell.h"
#include "AssetTree.h"

class SceneFixture;

namespace ui {

class AssetGridViewerChild
{
public:
	struct SpriteAssetGridSelection
	{
		std::string spriteSeries;
		Sprite sprite;
		bool viewingInsideSeries = false;

		bool SpriteSeriesSelectedAtTopLevel() const
		{
			return !spriteSeries.empty() && !sprite.resourceHandle.IsValid() && !viewingInsideSeries;
		}

		bool HasSelection() const
		{
			return (!spriteSeries.empty() || sprite.resourceHandle.IsValid());
		}
	};

	struct ResourceContext
	{
		AssetItem::Type openAssetTabType = AssetItem::Type::Unknown;
		const AssetTree& assetTree;
		const AssetViewerIcons& icons;
		const GuiTextureConverter& converter;
	};

	void Draw(SceneFixture& fixture, ResourceContext& ctx);

	AssetItem::Type HandleAssetDragDropTarget(SceneFixture& fixture, ResourceContext& ctx);

private:
	void DrawSpriteAssetGrid(SpriteAtlas& loadTargetAtlas, ResourceContext& ctx);
	AssetItem::Type HandleDirectoryAssetDragDropTarget(const AssetItem& item, const AssetTree& assetTree,
													   SceneFixture& fixture);
	AssetItem::Type HandleSpriteAssetDragDropTarget(const AssetItem& item, SpriteAtlas& loadTargetAtlas,
													SDL_Renderer* renderer);

	AssetItem::Type activeAssetPanelType = AssetItem::Type::Unknown;
	SpriteAssetGridSelection spriteSelection_;
};

} // ui

#endif