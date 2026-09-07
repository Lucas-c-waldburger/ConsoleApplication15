#include "AssetGridViewerChild.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"

namespace ui {

namespace {

size_t GetPayloadAssetId(const std::string_view payloadName)
{
	if (const auto* payload = ImGui::AcceptDragDropPayload(payloadName.data()))
	{
		return *static_cast<size_t*>(payload->Data);
	}

	return std::numeric_limits<size_t>::max();
}

const AssetItem& GetPayloadAssetItem(const AssetTree& assetTree, const std::string_view payloadName)
{
	if (const auto* payload = ImGui::AcceptDragDropPayload(payloadName.data()))
	{
		const auto assetId = *static_cast<size_t*>(payload->Data);

		return (assetId < assetTree.assets.size())
			? assetTree.assets[assetId]
			: Null<AssetItem>();
	}

	return Null<AssetItem>();
}

} // unnamed

void AssetGridViewerChild::Draw(SceneFixture& fixture, ResourceContext& ctx)
{
	if (!ImGui::BeginTabBar("Tabs"))
	{
		return;
	}

	auto getTabFlags = [type = ctx.openAssetTabType](AssetItem::Type tabType) -> int {
		if (type != AssetItem::Type::Unknown && type != AssetItem::Type::Directory)
		{
			return ImGuiTabItemFlags_SetSelected;
		}
		return 0;
	};

	if (ImGui::BeginTabItem("Sprites", nullptr, getTabFlags(AssetItem::Type::Image)))
	{
		DrawSpriteAssetGrid(fixture.GetTextureRepository().GetSpriteAtlas(), ctx);

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

AssetItem::Type AssetGridViewerChild::HandleAssetDragDropTarget(SceneFixture& fixture, ResourceContext& ctx)
{
	AssetItem::Type lastLoadedAssetType = AssetItem::Type::Unknown;

	if (!ImGui::BeginDragDropTarget())
	{
		return lastLoadedAssetType;
	}

	if (const auto& item = GetPayloadAssetItem(ctx.assetTree, kDirectoryPayloadName))
	{
		assert(item.type == AssetItem::Type::Directory);

		lastLoadedAssetType = HandleDirectoryAssetDragDropTarget(item, ctx.assetTree, fixture);
	}
	else if (const auto& item = GetPayloadAssetItem(ctx.assetTree, kImagePayloadName))
	{
		assert(item.type == AssetItem::Type::Image);

		lastLoadedAssetType = HandleSpriteAssetDragDropTarget(
			item, 
			fixture.GetTextureRepository().GetSpriteAtlas(),
			fixture.GetRenderer());
	}

	ImGui::EndDragDropTarget();

	return lastLoadedAssetType;
}

void AssetGridViewerChild::DrawSpriteAssetGrid(SpriteAtlas& loadTargetAtlas, ResourceContext& ctx)
{
	constexpr float thumbnailSize = 64.0f;
	constexpr float cellWidth = 80.0f;
	const float cellHeight = thumbnailSize + 4.0f + ImGui::GetTextLineHeightWithSpacing();
	const float panelWidth = ImGui::GetContentRegionAvail().x;

	const int columns = std::max(1, static_cast<int>(panelWidth / cellWidth));

	if (!ImGui::BeginTable("Sprite Asset Grid", columns))
	{
		return;
	}

	if (!spriteSelection_.spriteSeries.empty() &&
		spriteSelection_.viewingInsideSeries)
	{
		auto sprites = loadTargetAtlas.GetSpriteSeries(spriteSelection_.spriteSeries);
		if (sprites.empty())
		{
			spriteSelection_.spriteSeries.clear();
			spriteSelection_.sprite = {};
		}

		for (size_t i = 0; i < sprites.size(); ++i)
		{
			ImGui::TableNextColumn();
			ImGui::PushID(static_cast<int>(i));

			auto& sprite = sprites[i];

			const auto gridCell = AssetGridCell::Place();

			if (gridCell.Clicked())
			{
				spriteSelection_.sprite = sprite;
			}

			auto tx = ctx.converter.FromSprite(sprite);
			assert(tx.textureId != 0);

			gridCell.DrawThumbnailTexture(tx);

			if (spriteSelection_.sprite == sprite)
			{
				gridCell.DrawSelectedHighlight();
			}

			auto spriteNameOp = loadTargetAtlas.GetSpriteInfo<&SpriteInfo::spriteName>(sprite);
			assert(spriteNameOp.has_value());

			gridCell.DrawDisplayText(*spriteNameOp);

			ImGui::PopID();
		}
	}
	else
	{
		const auto seriesNames = loadTargetAtlas.GetSpriteSeriesNames();
		if (!seriesNames.empty())
		{
			for (size_t i = 0; i < seriesNames.size(); ++i)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(static_cast<int>(i));

				const auto gridCell = AssetGridCell::Place();

				if (gridCell.Clicked())
				{
					spriteSelection_.spriteSeries = seriesNames[i];
					spriteSelection_.sprite = {};
				}

				auto tx = ctx.converter.FromSprite(ctx.icons.folderClosedLargeSprite);
				assert(tx.textureId != 0);

				gridCell.DrawThumbnailTexture(tx);

				if (spriteSelection_.spriteSeries == seriesNames[i])
				{
					gridCell.DrawSelectedHighlight();
				}

				gridCell.DrawDisplayText(seriesNames[i]);

				ImGui::PopID();
			}
		}

		auto it = loadTargetAtlas.IterSpriteInfo<&SpriteInfo::filepath,
												 &SpriteInfo::spriteName,
												 &SpriteInfo::plot,
												 &SpriteInfo::atlasId,
												 &SpriteInfo::generation>();
		size_t counter = 0;
		for (const auto [filepath, spriteName, plot, atlasId, gen] : it)
		{
			size_t i = counter++;

			if (filepath.empty())
			{
				continue;
			}

			ImGui::TableNextColumn();
			ImGui::PushID(static_cast<int>(i));

			auto handle = Handle<TextureResource>::Create(atlasId, i, gen);

			const auto gridCell = AssetGridCell::Place();

			if (gridCell.Clicked())
			{
				spriteSelection_.spriteSeries.clear();
				spriteSelection_.sprite.resourceHandle = handle;
				spriteSelection_.sprite.plot = plot;
			}

			auto tx = ctx.converter.FromTextureResource(handle, plot);
			assert(tx.textureId != 0);

			gridCell.DrawThumbnailTexture(tx);

			if (spriteSelection_.sprite.resourceHandle == handle &&
				spriteSelection_.sprite.plot == plot)
			{
				gridCell.DrawSelectedHighlight();
			}

			gridCell.DrawDisplayText(spriteName);

			ImGui::PopID();
		}
	}

	ImGui::EndTable();
}

AssetItem::Type AssetGridViewerChild::HandleDirectoryAssetDragDropTarget(const AssetItem& item, 
																		 const AssetTree& assetTree,
																		 SceneFixture& fixture)
{
	AssetItem::Type lastLoadedAssetType = AssetItem::Type::Unknown;

	for (const auto fileChildId : item.fileTypeChildren)
	{
		if (fileChildId >= assetTree.assets.size())
		{
			continue;
		}

		const auto& childItem = assetTree.assets[fileChildId];

		switch (childItem.type)
		{
		case AssetItem::Type::Image:
			lastLoadedAssetType = HandleSpriteAssetDragDropTarget(
				item,
				fixture.GetTextureRepository().GetSpriteAtlas(),
				fixture.GetRenderer());

			break;
		}
	}

	return lastLoadedAssetType;
}

AssetItem::Type AssetGridViewerChild::HandleSpriteAssetDragDropTarget(const AssetItem& item,
																	  SpriteAtlas& loadTargetAtlas,
																	  SDL_Renderer* renderer)
{
	assert(renderer);

	LOG_IF_ERROR(loadTargetAtlas.LoadSprite(renderer, { .filepath = item.path.string() }));

	return AssetItem::Type::Image;
}

} // ui

#endif