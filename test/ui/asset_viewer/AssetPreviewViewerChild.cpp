#include "AssetPreviewViewerChild.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"

namespace ui {

void AssetPreviewViewerChild::Draw(SceneFixture& fixture, ResourceContext& ctx)
{
	if (ctx.spriteSelection.HasSelection())
	{
		DrawSpriteAssetPreview(
			ctx.spriteSelection,
			ctx.converter,
			fixture.GetTextureRepository().GetSpriteAtlas());
	}
}


void AssetPreviewViewerChild::DrawSpriteAssetPreview(const SpriteAssetGridSelection& spriteSelection, 
													 const GuiTextureConverter& converter,
													 const SpriteAtlas& spriteAtlas)
{
	if (spriteSelection.SpriteSeriesSelectedAtTopLevel())
	{
		auto sprites = spriteAtlas.GetSpriteSeries(spriteSelection.spriteSeries);

		for (size_t i = 0; i < sprites.size(); ++i)
		{
			DrawSpriteAssetSinglePreview(sprites[i], converter, spriteAtlas);

			if (i < sprites.size() - 1)
			{
				ImGui::SameLine();
			}
		}
	}
	else if (spriteAtlas.IsSpriteValid(spriteSelection.sprite))
	{
		DrawSpriteAssetSinglePreview(spriteSelection.sprite, converter, spriteAtlas);
	}
}

void AssetPreviewViewerChild::DrawSpriteAssetSinglePreview(const Sprite& sprite, 
														   const GuiTextureConverter& converter,
														   const SpriteAtlas& spriteAtlas)
{
	auto tx = converter.FromTextureResource(sprite.resourceHandle, sprite.plot);
	assert(tx.textureId != 0);

	GuiImage(tx);

	const auto info = spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName,
												&SpriteInfo::filepath>(sprite);
	assert(info.has_value());

	const auto& [spriteName, filepath] = *info;

	ImGui::TextUnformatted(spriteName.c_str());

	ImGui::TextUnformatted(filepath.c_str());

	const auto dimensionsStr = std::format("{} x {}", sprite.plot.rect.w, sprite.plot.rect.h);

	ImGui::TextUnformatted(dimensionsStr.c_str());
}

} // ui

#endif