#pragma once
#include "AssetGridViewerChild.h"

#if IMGUI_ENABLED

namespace ui {

class AssetPreviewViewerChild
{
public:
	using SpriteAssetGridSelection = AssetGridViewerChild::SpriteAssetGridSelection;

	struct ResourceContext
	{
		const SpriteAssetGridSelection& spriteSelection;
		const GuiTextureConverter& converter;
	};

	static void Draw(SceneFixture& fixture, ResourceContext& ctx);

private:
	AssetPreviewViewerChild() = default;

	static void DrawSpriteAssetPreview(const SpriteAssetGridSelection& spriteSelection, 
									   const GuiTextureConverter& converter,
									   const SpriteAtlas& spriteAtlas);

	static void DrawSpriteAssetSinglePreview(const Sprite& sprite, const GuiTextureConverter& converter,
											 const SpriteAtlas& spriteAtlas);
};


} // ui

#endif
