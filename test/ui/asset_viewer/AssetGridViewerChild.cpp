#include "AssetGridViewerChild.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"

namespace ui {

namespace {

constexpr float kThumbnailTextureSize = 64.0f;
constexpr float kGridCellWidth = 80.0f;

int GetGridColumnCount()
{
	const float cellHeight = kThumbnailTextureSize + 4.0f + ImGui::GetTextLineHeightWithSpacing();
	const float panelWidth = ImGui::GetContentRegionAvail().x;

	return std::max(1, static_cast<int>(panelWidth / kGridCellWidth));
}

Result<Sprite> RenameSpriteAndUpdateEntities(std::string_view oldName, std::string_view newName,
											 SDL_Renderer* renderer, SpriteAtlas& loadTargetAtlas)
{
	assert(renderer);

	if (newName.empty())
	{
		return Sprite{};
	}
	if (loadTargetAtlas.HasSprite(newName))
	{
		return MAKE_ERROR_FMT("New sprite name '{}' already exists in atlas", newName);
	}

	const auto sprite = loadTargetAtlas.GetSprite(oldName);
	if (!sprite.resourceHandle.IsValid())
	{
		return sprite;
	}

	const auto info = loadTargetAtlas.GetSpriteInfo<&SpriteInfo::filepath>(sprite);
	assert(info.has_value());

	const auto filepath = *info;
	assert(!filepath.empty());

	auto esToUpdate = ECS::GetAllEntitiesWith<SpriteRenderableComponent>(
		[handle = sprite.resourceHandle](auto& r) { return r.sprite.resourceHandle == handle; });

	[[maybe_unused]] const bool erased = loadTargetAtlas.EraseSprite(sprite);
	assert(erased);

	TRY(loadTargetAtlas.LoadSprite(renderer, { .filepath = std::move(filepath) }), newSprite);
	assert(newSprite.resourceHandle.IsValid());

	for (auto& e : esToUpdate)
	{
		e.GetComponent<SpriteRenderableComponent>().sprite = newSprite;
	}

	return newSprite;
}

Result<Void> RenameSpriteSeriesAndUpdateEntities(std::string_view oldName, std::string_view newName, 
												 SpriteAtlas& loadTargetAtlas)
{
	if (newName.empty())
	{
		return kVoid;
	}
	if (loadTargetAtlas.HasSpriteSeries(newName))
	{
		return MAKE_ERROR_FMT("New sprite series name '{}' already exists in atlas", newName);
	}

	auto memberSprites = loadTargetAtlas.GetSpriteSeries(oldName);

	[[maybe_unused]] const bool removed = loadTargetAtlas.RemoveSpriteSeries(oldName);
	assert(removed);

	TRY(loadTargetAtlas.DefineSpriteSeries(newName, memberSprites));

	auto esToUpdate = ECS::GetAllEntitiesWith<SpriteAnimationComponent>([oldName](const auto& anim) {
		return anim.spriteSeriesName == oldName;
	});

	for (auto& e : esToUpdate)
	{
		e.GetComponent<SpriteAnimationComponent>().spriteSeriesName = newName;
	}

	return kVoid;
}

constexpr bool IsAssetItemTypeFile(AssetItem::Type type) noexcept
{
	return type != AssetItem::Type::Unknown && type != AssetItem::Type::Directory;
}

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

	auto getTabFlags = [type = openAssetTabType_](AssetItem::Type tabType) -> int {
		if (IsAssetItemTypeFile(type))
		{
			return ImGuiTabItemFlags_SetSelected;
		}
		return 0;
	};

	if (ImGui::BeginTabItem("Sprites", nullptr, getTabFlags(AssetItem::Type::Image)))
	{
		DrawSpriteAssetGrid(fixture.GetTextureRepository(), ctx);

		ResolveSpritePopupContextActions(fixture.GetTextureRepository().GetSpriteAtlas());

		openAssetTabType_ = AssetItem::Type::Image;

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

void AssetGridViewerChild::HandleAssetDragDropTarget(SceneFixture& fixture, ResourceContext& ctx)
{
	AssetItem::Type lastLoadedAssetType = AssetItem::Type::Unknown;

	if (!ImGui::BeginDragDropTarget())
	{
		return;
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

	if (IsAssetItemTypeFile(lastLoadedAssetType))
	{
		openAssetTabType_ = lastLoadedAssetType;
	}
}

void AssetGridViewerChild::DrawSpriteAssetGrid(TextureRepository& loadTargetRepo, ResourceContext& ctx)
{
	if (!ImGui::BeginTable("Sprite Asset Grid", GetGridColumnCount()))
	{
		return;
	}

	GuiTextureConverter loadTargetConverter{ loadTargetRepo };

	int guiId = 0;

	if (!spriteSelection_.spriteSeries.empty() &&
		(spriteSelection_.state & GridSelectionState::ViewingInsideSeries))
	{
		auto sprites = loadTargetRepo.GetSpriteAtlas().GetSpriteSeries(spriteSelection_.spriteSeries);
		if (sprites.empty())
		{
			spriteSelection_.spriteSeries.clear();
			spriteSelection_.sprite = {};
		}

		for (const auto& sprite : sprites)
		{
			ImGui::TableNextColumn();
			ImGui::PushID(guiId);

			const auto gridCell = AssetGridCell::Place();

			if (gridCell.Clicked())
			{
				if (spriteSelection_.sprite != sprite)
				{
					spriteSelection_.ClearRename();
					spriteSelection_.sprite = sprite;
				}
			}

			bool alreadyRenaming = spriteSelection_.IsRenaming();

			DrawSpritePopupContextMenu(loadTargetRepo.GetSpriteAtlas());

			const bool currentCellSelected = spriteSelection_.sprite == sprite;
			if (currentCellSelected)
			{
				gridCell.DrawSelectedHighlight();
			}

			auto tx = loadTargetConverter.FromSprite(sprite);
			assert(tx.textureId != 0);

			gridCell.DrawThumbnailTexture(tx);

			auto spriteNameOp = 
				loadTargetRepo.GetSpriteAtlas().GetSpriteInfo<&SpriteInfo::spriteName>(sprite);
			assert(spriteNameOp.has_value());

			if (currentCellSelected && spriteSelection_.IsRenaming())
			{
				HandleSpriteSelectionRename(gridCell, loadTargetRepo.GetSpriteAtlas(),
											!alreadyRenaming);
			}
			else
			{
				gridCell.DrawDisplayText(*spriteNameOp);
			}

			ImGui::PopID();

			++guiId;
		}
	}
	else
	{
		const auto seriesNames = loadTargetRepo.GetSpriteAtlas().GetSpriteSeriesNames();
		if (!seriesNames.empty())
		{
			for (const auto& seriesName : seriesNames)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(guiId);

				const auto gridCell = AssetGridCell::Place();

				if (gridCell.Clicked())
				{
					if (spriteSelection_.spriteSeries != seriesName)
					{
						spriteSelection_.ClearRename();
						spriteSelection_.spriteSeries = seriesName;
						spriteSelection_.sprite = {};
					}
				}

				bool alreadyRenaming = spriteSelection_.IsRenaming();

				DrawSpritePopupContextMenu(loadTargetRepo.GetSpriteAtlas());

				const bool currentCellSelected = spriteSelection_.spriteSeries == seriesName;
				if (currentCellSelected)
				{
					gridCell.DrawSelectedHighlight();
				}

				auto tx = ctx.uiTexturesConverter.FromSprite(ctx.icons.mediaFolderLargeSprite);
				assert(tx.textureId != 0);

				gridCell.DrawThumbnailTexture(tx);

				if (currentCellSelected && spriteSelection_.IsRenaming())
				{
					HandleSpriteSelectionRename(gridCell, loadTargetRepo.GetSpriteAtlas(),
												!alreadyRenaming);
				}
				else
				{
					gridCell.DrawDisplayText(seriesName);
				}

				ImGui::PopID();

				++guiId;
			}
		}

		auto it = loadTargetRepo.GetSpriteAtlas().IterSpriteInfo<&SpriteInfo::filepath,
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
			ImGui::PushID(guiId);

			Sprite sprite{
				.resourceHandle = Handle<TextureResource>::Create(atlasId, i, gen),
				.plot = plot
			};

			const auto gridCell = AssetGridCell::Place();

			if (gridCell.Clicked())
			{
				if (spriteSelection_.sprite != sprite)
				{
					spriteSelection_.ClearRename();
					spriteSelection_.spriteSeries.clear();
					spriteSelection_.sprite = sprite;
				}
			}

			bool alreadyRenaming = spriteSelection_.IsRenaming();

			DrawSpritePopupContextMenu(loadTargetRepo.GetSpriteAtlas());

			const bool currentCellSelected = spriteSelection_.sprite == sprite;
			if (currentCellSelected)
			{
				gridCell.DrawSelectedHighlight();
			}

			auto tx = loadTargetConverter.FromSprite(sprite);
			assert(tx.textureId != 0);

			gridCell.DrawThumbnailTexture(tx);

			if (currentCellSelected && spriteSelection_.IsRenaming())
			{
				HandleSpriteSelectionRename(gridCell, loadTargetRepo.GetSpriteAtlas(),
											!alreadyRenaming);
			}
			else
			{
				gridCell.DrawDisplayText(spriteName);
			}

			ImGui::PopID();

			++guiId;
		}
	}

	ImGui::EndTable();
}

void AssetGridViewerChild::DrawAudioAssetGrid(AudioBank& audioBank, ResourceContext& ctx)
{
	if (!ImGui::BeginTable("Audio Asset Grid", GetGridColumnCount()))
	{
		return;
	}

	int guiId = 0;
	auto it = audioBank.IterAudioInfo<&AudioInfo::audioType, &AudioInfo::name>();
	for (const auto [audioType, audioName] : it)
	{
		ImGui::TableNextColumn();
		ImGui::PushID(guiId);

		auto gridCell = AssetGridCell::Place();

		if (gridCell.Clicked() && audioSelection_.audioName != audioName)
		{
			audioSelection_.audioName = audioName;
		}

		bool alreadyRenaming = audioSelection_.IsRenaming();

		DrawAudioPopupContextMenu(audioBank);

		const bool currentCellSelected = audioSelection_.audioName == audioName;
		if (currentCellSelected)
		{
			gridCell.DrawSelectedHighlight();
		}

		const auto& sprite = (audioType == AudioType::Music)
			? ctx.icons.musicFileLargeSprite
			: ctx.icons.soundFileLargeSprite;

		auto tx = ctx.uiTexturesConverter.FromSprite(sprite);
		assert(tx.textureId != 0);

		gridCell.DrawThumbnailTexture(tx);

		++guiId;
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

void AssetGridViewerChild::DrawSpritePopupContextMenu(SpriteAtlas& loadTargetAtlas)
{
	if (!spriteSelection_.HasSelection())
	{
		return;
	}

	if (ImGui::BeginPopupContextItem("SpriteThumbnailContextMenu"))
	{
		if (ImGui::MenuItem("Rename"))
		{
			spriteSelection_.currentRename.clear();

			if (spriteSelection_.sprite.resourceHandle.IsValid())
			{
				auto spriteName =
					loadTargetAtlas.GetSpriteInfo<&SpriteInfo::spriteName>(spriteSelection_.sprite);
				assert(spriteName.has_value());

				spriteSelection_.currentRename = *spriteName;
			}
			else if (!spriteSelection_.spriteSeries.empty())
			{
				spriteSelection_.currentRename = spriteSelection_.spriteSeries;
			}

			spriteSelection_.state |= GridSelectionState::Renaming;
		}

		if (ImGui::MenuItem("Erase"))
		{
			spriteSelection_.state |= GridSelectionState::MarkedErase;
		}

		ImGui::EndPopup();
	}
}

void AssetGridViewerChild::DrawAudioPopupContextMenu(AudioBank& audioBank)
{

}

void AssetGridViewerChild::ResolveSpritePopupContextActions(SpriteAtlas& loadTargetAtlas)
{
	if (spriteSelection_.state & GridSelectionState::MarkedErase)
	{
		if (loadTargetAtlas.IsSpriteValid(spriteSelection_.sprite))
		{
			const bool erased = loadTargetAtlas.EraseSprite(spriteSelection_.sprite);
			if (!erased)
			{
				LOG_ERROR("Failed to erase sprite");
			}
			else
			{
				spriteSelection_.sprite = {};
			}
		}
		else if (loadTargetAtlas.HasSpriteSeries(spriteSelection_.spriteSeries))
		{
			const bool removed = loadTargetAtlas.RemoveSpriteSeries(spriteSelection_.spriteSeries);
			if (!removed)
			{
				LOG_ERROR_FMT("Failed to remove sprite series '{}'", spriteSelection_.spriteSeries);
			}
			else
			{
				spriteSelection_.spriteSeries.clear();
				spriteSelection_.state &= ~GridSelectionState::ViewingInsideSeries;
			}
		}

		spriteSelection_.currentRename.clear();
		spriteSelection_.state &= ~(GridSelectionState::MarkedErase |
									GridSelectionState::Renaming);
	}
}

void AssetGridViewerChild::HandleSpriteSelectionRename(const AssetGridCell& gridCell, 
													   SpriteAtlas& loadTargetAtlas,
													   bool renameStartedThisFrame)
{
	assert(spriteSelection_.IsRenaming());

	const auto outcome = gridCell.DrawDisplayTextRenaming(spriteSelection_.currentRename,
														  renameStartedThisFrame);

	if (outcome == AssetGridCell::RenameOutcome::Continue)
	{
		return;
	}

	if (outcome == AssetGridCell::RenameOutcome::Complete)
	{
		if (loadTargetAtlas.IsSpriteValid(spriteSelection_.sprite))
		{
			LOG_IF_ERROR(loadTargetAtlas.SetSpriteName(spriteSelection_.sprite, 
													   spriteSelection_.currentRename));
		}
		else if (loadTargetAtlas.HasSpriteSeries(spriteSelection_.spriteSeries))
		{
			LOG_IF_ERROR(RenameSpriteSeriesAndUpdateEntities(spriteSelection_.spriteSeries,
															 spriteSelection_.currentRename, 
															 loadTargetAtlas));
		}
	}

	spriteSelection_.currentRename.clear();
	spriteSelection_.state &= ~GridSelectionState::Renaming;
}

void AssetGridViewerChild::HandleAudioSelectionRename(const AssetGridCell& gridCell,
													  AudioBank& audioBank, 
													  bool renameStartedThisFrame)
{
	assert(audioSelection_.IsRenaming());

	const auto outcome = gridCell.DrawDisplayTextRenaming(audioSelection_.currentRename,
														  renameStartedThisFrame);

	if (outcome == AssetGridCell::RenameOutcome::Continue)
	{
		return;
	}

	if (outcome == AssetGridCell::RenameOutcome::Complete)
	{
		if (loadTargetAtlas.IsSpriteValid(spriteSelection_.sprite))
		{
			LOG_IF_ERROR(loadTargetAtlas.SetSpriteName(spriteSelection_.sprite,
				spriteSelection_.currentRename));
		}
		else if (loadTargetAtlas.HasSpriteSeries(spriteSelection_.spriteSeries))
		{
			LOG_IF_ERROR(RenameSpriteSeriesAndUpdateEntities(spriteSelection_.spriteSeries,
				spriteSelection_.currentRename,
				loadTargetAtlas));
		}
	}

	spriteSelection_.currentRename.clear();
	spriteSelection_.state &= ~GridSelectionState::Renaming;
}

} // ui

#endif