#include "AssetGridViewerChild.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"
#include "../../../audio/AudioBank.h"

namespace ui {

namespace {

constexpr float kThumbnailTextureSize = 64.0f;
constexpr float kGridCellWidth = 80.0f;

static inline constexpr std::string_view kSpriteGridCellPayloadName = "GRID_CELL_SPRITE";

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

	TRY(loadTargetAtlas.LoadSprite(renderer, { .filepath = filepath }), newSprite);
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

std::string_view GetAudioConvertPopupText(const Handle<Audio>& handle, const AudioBank& bank)
{
	static constexpr std::string_view kToMusic = "Convert to Music";
	static constexpr std::string_view kToSound = "Convert to Sound";

	auto audioType = bank.GetAudioInfo<&AudioInfo::audioType>(handle);
	if (audioType.has_value())
	{
		switch (*audioType)
		{
		case AudioType::Music: return kToSound;
		case AudioType::Sound: return kToMusic;
		}
	}

	return "";
}

void StopEntitiesWithAudio(const Handle<Audio>& handle)
{
	auto es = ECS::GetAllEntitiesWith<ActiveAudio>([&handle](const auto& aa) {
		return aa.audioHandle == handle;
	});

	for (auto& e : es)
	{
		auto& req = e.AddComponent<AudioUpdateRequest>();
		req.instanceId = e.GetComponent<ActiveAudio>().instanceId;
		req.command = AudioPlayCommand::Stop;
	}
}

bool ReadyToPerformAudioRequest(const AssetGridViewerChild::AudioAssetGridSelection& sel,
								uint8_t audioReqFlag)
{
	using enum AssetGridViewerChild::GridSelectionState;

	return (sel.state & (audioReqFlag | GameLoopPassedSinceRequest)) ==
		(audioReqFlag | GameLoopPassedSinceRequest);
}

} // unnamed

void AssetGridViewerChild::Draw(SceneFixture& fixture, ResourceContext& ctx)
{
	if (!ImGui::BeginTabBar("Tabs"))
	{
		return;
	}

	auto getTabFlags = [this](AssetItem::Type tabType) -> int {
		return (forceAssetTabOpen_ == tabType) ? ImGuiTabItemFlags_SetSelected : 0;
	};

	// Sprites
	if (ImGui::BeginTabItem("Sprites", nullptr, getTabFlags(AssetItem::Type::Image)))
	{
		if (ImGui::BeginPopupContextItem("SpriteTabItemMenu"))
		{
			DrawSpriteTabItemMenu(fixture.GetTextureRepository().GetSpriteAtlas());

			ImGui::EndPopup();
		}

		DrawSpriteAssetGrid(fixture.GetTextureRepository(), ctx);

		ResolveSpritePopupContextActions(fixture.GetTextureRepository().GetSpriteAtlas());

		currentAssetTab_ = AssetItem::Type::Image;

		ImGui::EndTabItem();
	}

	// Audio
	if (ImGui::BeginTabItem("Audio", nullptr, getTabFlags(AssetItem::Type::Audio)))
	{
		DrawAudioAssetGrid(fixture.GetAudioBank(), ctx);

		ResolveAudioPopupContextActions(fixture.GetAudioBank());

		currentAssetTab_ = AssetItem::Type::Audio;

		ImGui::EndTabItem();
	}

	//Fonts
	if (ImGui::BeginTabItem("Fonts", nullptr, getTabFlags(AssetItem::Type::Font)))
	{
		DrawFontAssetGrid(fixture.GetTextureRepository().GetFontAtlas(), ctx);

		ResolveFontPopupContextActions(fixture.GetTextureRepository().GetFontAtlas());

		currentAssetTab_ = AssetItem::Type::Font;

		ImGui::EndTabItem();
	}

	forceAssetTabOpen_.reset();

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
	else if (const auto& item = GetPayloadAssetItem(ctx.assetTree, kAudioPayloadName))
	{
		assert(item.type == AssetItem::Type::Audio);

		if (!pendingDragDropTargetItem_)
		{
			pendingDragDropTargetItem_ = item;

			ImGui::OpenPopup("loadAsMusOrSound");
		}
	}
	else if (const auto& item = GetPayloadAssetItem(ctx.assetTree, kFontPayloadName))
	{
		assert(item.type == AssetItem::Type::Font);

		lastLoadedAssetType = HandleFontAssetDragDropTarget(
			item,
			fixture.GetTextureRepository().GetFontAtlas(),
			fixture.GetRenderer());
	}

	ImGui::EndDragDropTarget();

	if (pendingDragDropTargetItem_)
	{
		lastLoadedAssetType = ResolvePendingAssetDragDropTarget(fixture, lastLoadedAssetType);
	}

	if (IsAssetItemTypeFile(lastLoadedAssetType))
	{
		forceAssetTabOpen_ = lastLoadedAssetType;
	}
}

AssetItem::Type AssetGridViewerChild::ResolvePendingAssetDragDropTarget(SceneFixture& fixture, 
																		AssetItem::Type lastLoadedType)
{
	assert(pendingDragDropTargetItem_);

	uint8_t outcome = 0;

	switch (pendingDragDropTargetItem_->type)
	{
	case AssetItem::Type::Audio:
		outcome = ResolveAudioAssetDragDropTarget(fixture.GetAudioBank());
		if (outcome & ResolveAssetDragDropOutcome::LoadSuccessful)
		{
			lastLoadedType = AssetItem::Type::Audio;
		}
		break;

	default:
		break;
	}

	if (outcome & ResolveAssetDragDropOutcome::Completed)
	{
		pendingDragDropTargetItem_.reset();
	}

	return lastLoadedType;
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
			spriteSelection_.Reset();
		}

		for (const auto& sprite : sprites)
		{
			ImGui::TableNextColumn();
			ImGui::PushID(guiId);

			const auto gridCell = AssetGridCell::Place();

			if (gridCell.Clicked() && spriteSelection_.sprite != sprite)
			{
				spriteSelection_.Reset();
				spriteSelection_.sprite = sprite;
			}

			bool alreadyRenaming = spriteSelection_.IsRenaming();

			DrawSpritePopupContextMenu(loadTargetRepo.GetSpriteAtlas());

			const bool currentCellSelected = spriteSelection_.sprite == sprite;
			if (currentCellSelected)
			{
				gridCell.DrawSelectedHighlight();

				if (spriteSelection_.state & GridSelectionState::JustDragDroppedIntoGrid)
				{
					ImGui::SetScrollHereY();
					spriteSelection_.state &= ~GridSelectionState::JustDragDroppedIntoGrid;
				}
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

				if (gridCell.Clicked() && spriteSelection_.spriteSeries != seriesName)
				{
					spriteSelection_.Reset();
					spriteSelection_.spriteSeries = seriesName;
				}

				if (HandleSpriteGridCellDragDropTarget(loadTargetRepo.GetSpriteAtlas(), seriesName))
				{
					spriteSelection_.Reset();
					spriteSelection_.spriteSeries = seriesName;
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

			ImGui::EndTable();

			ImGui::Separator();

			ImGui::BeginTable("Sprite Asset Grid 2", GetGridColumnCount());

			ImGui::TableNextRow();
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

			if (gridCell.Clicked() && spriteSelection_.sprite != sprite)
			{
				spriteSelection_.Reset();
				spriteSelection_.sprite = sprite;
			}

			HandleSpriteGridCellDragDropSource(loadTargetRepo.GetSpriteAtlas());

			bool alreadyRenaming = spriteSelection_.IsRenaming();

			DrawSpritePopupContextMenu(loadTargetRepo.GetSpriteAtlas());

			const bool currentCellSelected = spriteSelection_.sprite == sprite;
			if (currentCellSelected)
			{
				gridCell.DrawSelectedHighlight();

				if (spriteSelection_.state & GridSelectionState::JustDragDroppedIntoGrid)
				{
					ImGui::SetScrollHereY();
					spriteSelection_.state &= ~GridSelectionState::JustDragDroppedIntoGrid;
				}
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
	audioSelection_.UpdateGameLoopPassedFlag();

	if (!ImGui::BeginTable("Audio Asset Grid", GetGridColumnCount()))
	{
		return;
	}

	auto it = audioBank.IterAudioInfo<&AudioInfo::audioType, 
									  &AudioInfo::name, 
									  &AudioInfo::storageIndex,
									  &AudioInfo::generation>();
	size_t counter = 0;
	for (const auto [audioType, audioName, storageIdx, gen] : it)
	{
		if (audioName.empty())
		{
			++counter;
			continue; 
		}

		ImGui::TableNextColumn();
		ImGui::PushID(static_cast<int>(counter));

		auto gridCell = AssetGridCell::Place();

		auto handle = Handle<Audio>::Create(audioBank.GetBankID(), counter, gen);

		if (gridCell.Clicked())
		{
			if (audioSelection_.audioHandle != handle)
			{
				audioSelection_.Reset();
				audioSelection_.audioHandle = handle;
			}
		}

		bool alreadyRenaming = audioSelection_.IsRenaming();

		DrawAudioPopupContextMenu(audioBank);

		const bool currentCellSelected = audioSelection_.audioHandle == handle;
		if (currentCellSelected)
		{
			gridCell.DrawSelectedHighlight();

			if (audioSelection_.state & GridSelectionState::JustDragDroppedIntoGrid)
			{
				ImGui::SetScrollHereY();
				audioSelection_.state &= ~GridSelectionState::JustDragDroppedIntoGrid;
			}
		}

		const auto& sprite = (audioType == AudioType::Music)
			? ctx.icons.musicFileLargeSprite
			: ctx.icons.soundFileLargeSprite;

		auto tx = ctx.uiTexturesConverter.FromSprite(sprite);
		assert(tx.textureId != 0);

		gridCell.DrawThumbnailTexture(tx);

		if (currentCellSelected && audioSelection_.IsRenaming())
		{
			HandleAudioSelectionRename(gridCell, audioBank, !alreadyRenaming);
		}
		else
		{
			gridCell.DrawDisplayText(audioName);
		}

		ImGui::PopID();

		++counter;
	}

	ImGui::EndTable();
}

void AssetGridViewerChild::DrawFontAssetGrid(FontAtlas& fontAtlas, ResourceContext& ctx)
{
	if (!ImGui::BeginTable("Font Asset Grid", GetGridColumnCount()))
	{
		return;
	}

	auto it = fontAtlas.IterFontInfo<&FontInfo::fontName>();
	size_t counter = 0;
	bool rebuildGuiFonts = false;

	for (const auto& fontName : it)
	{
		const auto& handle = fontAtlas.GetResourceHandle(fontName);
		if (!handle.IsValid())
		{
			++counter;
			continue;
		}
		if (!fontSelection_.handleToGuiFont.contains(handle))
		{
			const auto infoOp = fontAtlas.GetFontInfo<&FontInfo::filepath,
													  &FontInfo::fontSize>(handle);
			assert(infoOp);

			const auto& [filepath, fontSize] = *infoOp;

			ImFont* guiFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(
				filepath.c_str(), static_cast<float>(fontSize));

			if (!guiFont)
			{
				LOG_ERROR_FMT("Could not load font '{}' to ImGui", filepath);
			}
			else
			{
				auto [_, inserted] = fontSelection_.handleToGuiFont.try_emplace(
					handle, guiFont
				);
				assert(inserted);

				rebuildGuiFonts = true;
			}
		}

		ImGui::TableNextColumn();
		ImGui::PushID(static_cast<int>(counter));

		auto gridCell = AssetGridCell::Place();

		if (gridCell.Clicked())
		{
			if (fontSelection_.fontHandle != handle)
			{
				fontSelection_.Reset();
				fontSelection_.fontHandle = handle;
			}
		}

		bool alreadyRenaming = fontSelection_.IsRenaming();

		DrawFontPopupContextMenu(fontAtlas);

		const bool currentCellSelected = fontSelection_.fontHandle == handle;
		if (currentCellSelected)
		{
			gridCell.DrawSelectedHighlight();

			if (fontSelection_.state & GridSelectionState::JustDragDroppedIntoGrid)
			{
				ImGui::SetScrollHereY();
				fontSelection_.state &= ~GridSelectionState::JustDragDroppedIntoGrid;
			}
		}

		const auto& sprite = ctx.icons.fontFileLargeSprite;

		auto tx = ctx.uiTexturesConverter.FromSprite(sprite);
		assert(tx.textureId != 0);

		gridCell.DrawThumbnailTexture(tx);

		if (currentCellSelected && fontSelection_.IsRenaming())
		{
			HandleFontSelectionRename(gridCell, fontAtlas, !alreadyRenaming);
		}
		else
		{
			gridCell.DrawDisplayText(fontName);
		}

		ImGui::PopID();

		++counter;
	}

	if (rebuildGuiFonts)
	{
		ImGui::GetIO().Fonts->Build();
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
				childItem,
				fixture.GetTextureRepository().GetSpriteAtlas(),
				fixture.GetRenderer());

			break;
		case AssetItem::Type::Audio:
			lastLoadedAssetType = HandleAudioAssetDragDropTarget(
				childItem,
				fixture.GetAudioBank());

			break;
		case AssetItem::Type::Font:
			lastLoadedAssetType = HandleFontAssetDragDropTarget(
				childItem,
				fixture.GetTextureRepository().GetFontAtlas(),
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

	auto spriteResult = loadTargetAtlas.LoadSprite(renderer, { .filepath = item.path.string() });
	if (!spriteResult.Success())
	{
		LOG_ERROR(spriteResult.GetError().GetMessage());
	}
	else
	{
		spriteSelection_.Reset();
		spriteSelection_.sprite = spriteResult.GetValue();
		spriteSelection_.state |= GridSelectionState::JustDragDroppedIntoGrid;
	}

	return AssetItem::Type::Image;
}

AssetItem::Type AssetGridViewerChild::HandleAudioAssetDragDropTarget(const AssetItem& item, 
																	 AudioBank& audioBank)
{
	ImGui::OpenPopup("loadAsMusOrSound");

	if (ImGui::BeginPopup("loadAsMusOrSound"))
	{
		AudioType reqAudioType = AudioType::Unknown;

		if (ImGui::MenuItem("As Music"))
		{
			reqAudioType = AudioType::Music;
		}
		if (ImGui::MenuItem("As Sound"))
		{
			reqAudioType = AudioType::Sound;
		}

		if (reqAudioType != AudioType::Unknown)
		{
			auto audioResult = audioBank.LoadAudio({
				.audioType = reqAudioType,
				.filepath = item.path.string()
			});
			if (!audioResult.Success())
			{
				LOG_ERROR(audioResult.GetError().GetMessage());
			}
			else
			{
				audioSelection_.Reset();
				audioSelection_.audioHandle = audioResult.GetValue();
				audioSelection_.state |= GridSelectionState::JustDragDroppedIntoGrid;
			}
		}

		ImGui::EndPopup();
	}

	return AssetItem::Type::Audio;
}

uint8_t AssetGridViewerChild::ResolveAudioAssetDragDropTarget(AudioBank& audioBank)
{
	assert(pendingDragDropTargetItem_); 

	if (ImGui::BeginPopup("loadAsMusOrSound"))
	{
		AudioType reqAudioType = AudioType::Unknown;

		if (ImGui::MenuItem("As Music"))
		{
			reqAudioType = AudioType::Music;
		}
		else if (ImGui::MenuItem("As Sound"))
		{
			reqAudioType = AudioType::Sound;
		}

		if (reqAudioType != AudioType::Unknown)
		{
			auto audioResult = audioBank.LoadAudio({
				.audioType = reqAudioType,
				.filepath = pendingDragDropTargetItem_->path.string()
			});
			if (!audioResult.Success())
			{
				LOG_ERROR(audioResult.GetError().GetMessage());

				return ResolveAssetDragDropOutcome::Completed;
			}
			else
			{
				audioSelection_.Reset();
				audioSelection_.audioHandle = audioResult.GetValue();
				audioSelection_.state |= GridSelectionState::JustDragDroppedIntoGrid;

				return (ResolveAssetDragDropOutcome::Completed |
					    ResolveAssetDragDropOutcome::LoadSuccessful);
			}
		}

		ImGui::EndPopup();

		return 0;
	}
	else
	{
		return ResolveAssetDragDropOutcome::Completed;
	}
}

AssetItem::Type AssetGridViewerChild::HandleFontAssetDragDropTarget(const AssetItem& item, 
																    FontAtlas& fontAtlas,
																	SDL_Renderer* renderer)
{
	auto pathStr = item.path.string();

	auto fontResult = fontAtlas.LoadFont(renderer, { 
		.filepath = pathStr,
		.fontSize = 16
	});

	if (!fontResult.Success())
	{
		LOG_ERROR(fontResult.GetError().GetMessage());
	}
	else
	{
		ImGuiIO& io = ImGui::GetIO();

		ImFont* guiFont = io.Fonts->AddFontFromFileTTF(pathStr.c_str(), 16.0f);

		io.Fonts->Build();

		if (!guiFont)
		{
			LOG_ERROR_FMT("Could not load font '{}' to ImGui", pathStr);
		}
		else
		{
			const auto& handle = fontResult.GetValue();

			auto fontNameOp = fontAtlas.GetFontInfo<&FontInfo::fontName>(handle);
			assert(fontNameOp);

			auto [_, inserted] = fontSelection_.handleToGuiFont.try_emplace(handle, guiFont);
			assert(inserted);

			fontSelection_.Reset();
			fontSelection_.fontHandle = handle;
			fontSelection_.state |= GridSelectionState::JustDragDroppedIntoGrid;
		}
	}

	return AssetItem::Type::Font;
}

void AssetGridViewerChild::HandleSpriteGridCellDragDropSource(const SpriteAtlas& loadTargetAtlas)
{
	if (ImGui::BeginDragDropSource())
	{
		if (!loadTargetAtlas.IsSpriteValid(spriteSelection_.sprite))
		{
			return;
		} 

		auto spriteName = loadTargetAtlas.GetSpriteInfo<&SpriteInfo::spriteName>(spriteSelection_.sprite);
		assert(spriteName);

		const size_t handleHash = spriteSelection_.sprite.resourceHandle.GetHash();

		ImGui::SetDragDropPayload(kSpriteGridCellPayloadName.data(), &handleHash, sizeof(handleHash));

		ImGui::TextUnformatted((*spriteName).c_str());

		ImGui::EndDragDropSource();
	}
}

bool AssetGridViewerChild::HandleSpriteGridCellDragDropTarget(SpriteAtlas& loadTargetAtlas,
															  std::string_view seriesName)
{
	if (!ImGui::BeginDragDropTarget())
	{
		return false;
	}

	bool complete = false;

	if (const auto* payload = ImGui::AcceptDragDropPayload(kSpriteGridCellPayloadName.data()))
	{
		const auto handleHash = *static_cast<size_t*>(payload->Data);

		if (handleHash == spriteSelection_.sprite.resourceHandle.GetHash() &&
			loadTargetAtlas.IsSpriteValid(spriteSelection_.sprite))
		{
			assert(loadTargetAtlas.HasSpriteSeries(seriesName));

			auto existingSeriesSprites = loadTargetAtlas.GetSpriteSeries(seriesName);
			existingSeriesSprites.emplace_back(spriteSelection_.sprite);

			loadTargetAtlas.DefineSpriteSeries(seriesName, existingSeriesSprites);

			complete = true;
		}
	}

	ImGui::EndDragDropTarget();

	return complete;
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
			spriteSelection_.state |= GridSelectionState::RequestErase;
		}

		ImGui::EndPopup();
	}
}

void AssetGridViewerChild::DrawAudioPopupContextMenu(AudioBank& audioBank)
{
	if (!audioSelection_.HasSelection())
	{
		return;
	}

	if (ImGui::BeginPopupContextItem("AudioThumbnailContextMenu"))
	{
		if (ImGui::MenuItem("Rename"))
		{
			audioSelection_.currentRename.clear();

			auto audioName = audioBank.GetAudioInfo<&AudioInfo::name>(audioSelection_.audioHandle);
			assert(audioName.has_value());
			assert(!(*audioName).empty());

			audioSelection_.currentRename = audioName;
			audioSelection_.state |= GridSelectionState::Renaming;
		}

		if (ImGui::MenuItem("Erase"))
		{
			audioSelection_.state |= GridSelectionState::RequestErase;

			StopEntitiesWithAudio(audioSelection_.audioHandle);
		}
		
		std::string_view convertPopupText =
			GetAudioConvertPopupText(audioSelection_.audioHandle, audioBank);
		assert(!convertPopupText.empty());

		if (ImGui::MenuItem(convertPopupText.data()))
		{
			audioSelection_.state |= GridSelectionState::RequestConvertAudioType;

			StopEntitiesWithAudio(audioSelection_.audioHandle);
		}

		ImGui::EndPopup();
	}
}

void AssetGridViewerChild::DrawFontPopupContextMenu(FontAtlas& fontAtlas)
{
	if (!fontSelection_.HasSelection())
	{
		return;
	}

	if (ImGui::BeginPopupContextItem("FontThumbnailContextMenu"))
	{
		if (ImGui::MenuItem("Rename"))
		{
			fontSelection_.currentRename.clear();

			if (fontSelection_.fontHandle.IsValid())
			{
				auto fontName =
					fontAtlas.GetFontInfo<&FontInfo::fontName>(fontSelection_.fontHandle);
				assert(fontName.has_value());

				fontSelection_.currentRename = *fontName;
			}

			spriteSelection_.state |= GridSelectionState::Renaming;
		}

		if (ImGui::MenuItem("Erase"))
		{
			fontSelection_.state |= GridSelectionState::RequestErase;
		}

		ImGui::EndPopup();
	}
}

void AssetGridViewerChild::DrawSpriteTabItemMenu(SpriteAtlas& loadTargetAtlas)
{
	if (ImGui::MenuItem("New Series"))
	{
		std::string newSeriesDisplayName;
		size_t i = 0;

		while (loadTargetAtlas.HasSpriteSeries(
			   newSeriesDisplayName = std::format("New Series ({})", i)))
		{
			++i;
		}

		LOG_IF_ERROR(loadTargetAtlas.DefineSpriteSeries(newSeriesDisplayName, {}));
	}
}

void AssetGridViewerChild::ResolveSpritePopupContextActions(SpriteAtlas& loadTargetAtlas)
{
	if (spriteSelection_.state & GridSelectionState::RequestErase)
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
		spriteSelection_.state &= ~(GridSelectionState::RequestErase |
									GridSelectionState::Renaming);
	}
}

void AssetGridViewerChild::ResolveAudioPopupContextActions(AudioBank& audioBank)
{
	if (ReadyToPerformAudioRequest(audioSelection_, GridSelectionState::RequestConvertAudioType))
	{
		if (audioBank.IsAudioValid(audioSelection_.audioHandle))
		{
			const auto info = audioBank.GetAudioInfo<&AudioInfo::audioType,
													 &AudioInfo::name,
													 &AudioInfo::filepath>(audioSelection_.audioHandle);
			assert(info.has_value());

			const auto [audioType, name, filepath] = *info;
			assert(audioType == AudioType::Music || audioType == AudioType::Sound);

			const auto flippedAudioType = (audioType == AudioType::Music)
				? AudioType::Sound
				: AudioType::Music;

			AudioDescriptor newDescriptor{
				.audioType = flippedAudioType,
				.name = name,
				.filepath = filepath
			};

			const bool erased = audioBank.EraseAudio(audioSelection_.audioHandle);
			audioSelection_.audioHandle = {};

			if (!erased)
			{
				LOG_ERROR("Failed to erase audio");
			}
			else
			{
				auto reloadResult = audioBank.LoadAudio(std::move(newDescriptor));
				if (!reloadResult.Success())
				{
					LOG_ERROR(reloadResult.GetError().GetMessage());
				}
				else
				{
					audioSelection_.audioHandle = reloadResult.GetValue();
				}
			}
		}

		audioSelection_.currentRename.clear();
		audioSelection_.state &= ~(GridSelectionState::RequestErase |
								   GridSelectionState::Renaming |
								   GridSelectionState::RequestConvertAudioType |
								   GridSelectionState::GameLoopPassedSinceRequest);
	}
	if (ReadyToPerformAudioRequest(audioSelection_, GridSelectionState::RequestErase))
	{
		if (audioBank.IsAudioValid(audioSelection_.audioHandle))
		{
			const bool erased = audioBank.EraseAudio(audioSelection_.audioHandle);
			audioSelection_.audioHandle = {};

			if (!erased)
			{
				LOG_ERROR("Failed to erase audio");
			}
		}

		audioSelection_.currentRename.clear();
		audioSelection_.state &= ~(GridSelectionState::RequestErase |
								   GridSelectionState::Renaming |
								   GridSelectionState::RequestConvertAudioType |
								   GridSelectionState::GameLoopPassedSinceRequest);
	}
}

void AssetGridViewerChild::ResolveFontPopupContextActions(FontAtlas& fontAtlas)
{
	if (fontSelection_.state & GridSelectionState::RequestErase)
	{
		if (fontAtlas.HasFont(fontSelection_.fontHandle))
		{
			const bool erased = fontAtlas.EraseFont(fontSelection_.fontHandle);
			if (!erased)
			{
				LOG_ERROR("Failed to erase font");
			}
			else
			{
				fontSelection_.handleToGuiFont.erase(fontSelection_.fontHandle);
				fontSelection_.fontHandle = {};
			}
		}

		fontSelection_.currentRename.clear();
		fontSelection_.state &= ~(GridSelectionState::RequestErase |
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
		if (audioBank.IsAudioValid(audioSelection_.audioHandle))
		{
			LOG_IF_ERROR(audioBank.SetAudioName(audioSelection_.audioHandle, 
											    audioSelection_.currentRename));
		}
	}

	audioSelection_.currentRename.clear();
	audioSelection_.state &= ~GridSelectionState::Renaming;
}

void AssetGridViewerChild::HandleFontSelectionRename(const AssetGridCell& gridCell, 
													 FontAtlas& fontAtlas, bool renameStartedThisFrame)
{
	assert(fontSelection_.IsRenaming());

	const auto outcome = gridCell.DrawDisplayTextRenaming(fontSelection_.currentRename,
														  renameStartedThisFrame);

	if (outcome == AssetGridCell::RenameOutcome::Continue)
	{
		return;
	}

	if (outcome == AssetGridCell::RenameOutcome::Complete)
	{
		if (fontAtlas.HasFont(fontSelection_.fontHandle))
		{
			LOG_IF_ERROR(fontAtlas.SetFontName(fontSelection_.fontHandle,
						 fontSelection_.currentRename));
		}
	}

	fontSelection_.currentRename.clear();
	fontSelection_.state &= ~GridSelectionState::Renaming;
}

} // ui

#endif