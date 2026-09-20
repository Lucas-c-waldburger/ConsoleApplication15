#include "AssetPreviewViewerChild.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"
#include "AudioPlayerContext.h"
#include "AssetViewerUtil.h"
#include <misc/cpp/imgui_stdlib.h>


namespace ui {

namespace {

static constexpr float kPreviewTextureSize = 256.0f;
static constexpr float kPreviewSeriesAnimationIndexSliderSize = 300.0f;
static constexpr float kPreviewSeriesAnimationSpeedSliderSize = 100.0f;
static constexpr float kPlayerButtonSize = 64.0f;
static constexpr float kPlayerButtonPadding = 15.0f;
static constexpr float kPlayerButtonsTotalWidth = kPlayerButtonSize * 3.0f;
static constexpr float kPreviewMusicVisualizerProgressBarSize = 300.0f;
static constexpr float kPreviewAudioVolumeSiderSize = 100.0f;
static constexpr float kPreviewAudioAngleSiderSize = 100.0f;
static constexpr float kPreviewAudioDistanceSiderSize = 100.0f;
static constexpr float kPreviewAudioPanningSiderSize = 100.0f;
static constexpr float kPreviewFontTextInputSize = 700.0f;
static constexpr float kPreviewSpriteSeriesReorderLayoutThumbnailSize = 64.0f;

void DrawPlayerSliderText(std::string_view txt, float sliderWidth)
{
	const float textWidth = ImGui::CalcTextSize(txt.data()).x;

	const float sliderPos = std::max(
		(ImGui::GetContentRegionAvail().x - sliderWidth) * 0.5f,
		0.0f);

	const float textStartX = sliderPos - ImGui::GetStyle().ItemSpacing.x - textWidth;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textStartX);

	ImGui::TextUnformatted(txt.data());

	ImGui::SameLine();

	ImGui::SetNextItemWidth(sliderWidth);
}

void DrawPanningSlider(AudioPlayerContext::AudioSpatialDrawData& spatialDrawData, 
					   std::optional<AudioPlayerContext>& ctx)
{
	const float textLWidth = ImGui::CalcTextSize("L").x;

	const float sliderPos = std::max(
		(ImGui::GetContentRegionAvail().x - kPreviewAudioPanningSiderSize) * 0.5f,
		0.0f);

	const float textLStartX = sliderPos - ImGui::GetStyle().ItemSpacing.x - textLWidth;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textLStartX);

	ImGui::TextUnformatted("L");

	ImGui::SameLine();

	ImGui::SetNextItemWidth(kPreviewAudioPanningSiderSize);

	if (ImGui::DragInt("##panning", &spatialDrawData.panning, 1.0f, -127, 127))
	{
		if (ctx)
		{
			const uint8_t left = static_cast<uint8_t>(
				std::clamp(128 - spatialDrawData.panning, 0, 255)
			);
			const uint8_t right = 255 - left;

			auto& updatePan = ctx->updateRequest.settings.spatial.panning;
			updatePan = AudioSpatialData::Panning{ left, right };
		}
	}

	ImGui::SameLine();

	ImGui::TextUnformatted("R");
}

void DrawElapsedTimeText(std::optional<AudioPlayerContext>& ctx)
{
	const auto timeText = (ctx)
		? ctx->MakeElapsedTimeText()
		: "0:00 / 0:00";

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(timeText.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(timeText.c_str());
}

void DrawVolumeSlider(std::optional<AudioPlayerContext>& ctx)
{
	ImGui::BeginDisabled(!ctx);

	DrawPlayerSliderText("volume:", kPreviewAudioVolumeSiderSize);

	int volume = (ctx) ? ctx->GetVolume() : 0;
	if (ImGui::DragInt("##volume", &volume, 1, 0, MIX_MAX_VOLUME))
	{
		if (ctx) { ctx->updateRequest.settings.volume = volume; }
	}

	ImGui::EndDisabled();
}

void DrawSpatialDataSliders(std::optional<AudioPlayerContext>& ctx)
{
	ImGui::BeginDisabled(!ctx || ctx->audioType != AudioType::Sound);

	auto spatial = (ctx) ? ctx->GetSpatialData() : AudioPlayerContext::AudioSpatialDrawData{};

	DrawPlayerSliderText("angle:", kPreviewAudioAngleSiderSize);

	if (ImGui::DragInt("##angle", &spatial.angle, 1.0f, 0, 360))
	{
		if (ctx) { ctx->updateRequest.settings.spatial.angle = static_cast<int16_t>(spatial.angle); }
	}

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	DrawPlayerSliderText("distance:", kPreviewAudioDistanceSiderSize);

	if (ImGui::DragInt("##distance", &spatial.distance, 1.0f, 0, 255))
	{
		if (ctx) { ctx->updateRequest.settings.spatial.distance = static_cast<uint8_t>(spatial.distance); }
	}

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	DrawPanningSlider(spatial, ctx);

	ImGui::EndDisabled();
}

} // unnamed

std::optional<AudioPlayerContext> 
AssetPreviewViewerChild::AudioPlayer::MakePlayerContext(const AudioBank& audioBank)
{
	auto e = ECS::GetEntityByID(entityId);
	if (!e.IsValid())
	{
		return std::nullopt;
	}

	return AudioPlayerContext::Create(e, audioBank);
}

void AssetPreviewViewerChild::AudioPlayer::Update(const Handle<Audio>& handle,
												  const AudioBank& audioBank)
{
	if (!audioBank.IsAudioValid(handle))
	{
		Reset();

		return;
	}

	auto e = ECS::GetEntityByID(entityId);
	if (!e.IsValid())
	{
		return;
	}

	if (e.HasComponent<ActiveAudio>())
	{
		const auto& aa = e.GetComponent<ActiveAudio>();

		if (aa.audioHandle != handle) // new audio selection
		{
			auto& newReq = e.AddComponent<NewAudioRequest>();
			newReq.audioHandle = handle;
			newReq.force = AudioForcing::ForcePausedAtStart;
			newReq.settings.spatial.distance = 0;
			newReq.settings.spatial.angle = 0;
			newReq.settings.spatial.panning = AudioSpatialData::Panning{ .left = 127, .right = 127 };

			musicVisualizer.Reset();
			state &= ~PlayerState::Playing;
		}
		else // check if play or pause requested
		{
			if (state & PlayerState::Playing)
			{
				if (aa.status == AudioStatus::Paused)
				{ 
					auto& updateReq = e.AddComponent<AudioUpdateRequest>();
					updateReq.instanceId = aa.instanceId;
					updateReq.command = AudioPlayCommand::Resume;
				}
			}
			else // if pause requested
			{
				if ((aa.status == AudioStatus::Playing || aa.status == AudioStatus::Stopping))
				{
					auto& updateReq = e.AddComponent<AudioUpdateRequest>();
					updateReq.instanceId = aa.instanceId;
					updateReq.command = AudioPlayCommand::Pause;
				}
			}
		}
	}
	else if (audioBank.IsAudioValid(handle)) // new audio selection
	{
		auto& newReq = e.AddComponent<NewAudioRequest>();
		newReq.audioHandle = handle;
		newReq.force = (AudioForcing::ForcePausedAtStart | AudioForcing::ForceChannelHalt);

		musicVisualizer.Reset();
		state &= ~PlayerState::Playing;
	}
}

void AssetPreviewViewerChild::AudioPlayer::Reset()
{
	if (auto e = ECS::GetEntityByID(entityId); e.HasComponent<ActiveAudio>())
	{
		const auto& aa = e.GetComponent<ActiveAudio>();
		auto& req = e.AddComponent<AudioUpdateRequest>();

		req.command = AudioPlayCommand::Halt;
		req.instanceId = aa.instanceId;
	}

	state &= ~PlayerState::Playing;
	musicVisualizer.Reset();
}

void AssetPreviewViewerChild::Draw(SceneFixture& fixture, ResourceContext& ctx, 
								   const DataRecord<AssetItem::Type>& assetGridTabType)
{
	if (assetGridTabType.last != assetGridTabType.now)
	{
		switch (assetGridTabType.last)
		{
		case AssetItem::Type::Image:
			spriteSeriesAnimator_.Reset();
			break;
		case AssetItem::Type::Audio:
			audioPlayer_.Reset();
			break;
		case AssetItem::Type::Font:
			break;
		default:
			break;
		}
	}

	if (assetGridTabType.now == AssetItem::Type::Image && 
		ctx.spriteSelection.HasSelection())
	{
		DrawSpriteAssetPreview(
			ctx,
			fixture.GetTextureRepository().GetSpriteAtlas(),
			fixture.GetDeltaTime());
	}
	else if (assetGridTabType.now == AssetItem::Type::Audio &&
		     ctx.audioSelection.HasSelection())
	{
		DrawAudioAssetPreview(
			ctx,
			fixture.GetAudioBank());
	}
	else if (assetGridTabType.now == AssetItem::Type::Font &&
			 ctx.fontSelection.HasSelection())
	{
		DrawFontAssetPreview(ctx);
	}
}


void AssetPreviewViewerChild::DrawSpriteAssetPreview(ResourceContext& ctx,
													 SpriteAtlas& spriteAtlas, float dt)
{
	if (ctx.spriteSelection.SpriteSeriesSelectedAtTopLevel())
	{
		auto sprites = spriteAtlas.GetSpriteSeries(ctx.spriteSelection.spriteSeries);

		DrawSpriteSeriesAssetsPreview(sprites, 
			ctx.spriteSelection.spriteSeries,
			ctx.loadTargetConverter, 
			ctx.uiTexturesConverter,
			spriteAtlas, dt);
	}
	else if (spriteAtlas.IsSpriteValid(ctx.spriteSelection.sprite))
	{
		DrawSpriteAssetSinglePreview(ctx.spriteSelection.sprite, ctx.loadTargetConverter, spriteAtlas);
	}
}

void AssetPreviewViewerChild::DrawAudioAssetPreview(ResourceContext& ctx, const AudioBank& audioBank)
{
	audioPlayer_.Update(ctx.audioSelection.audioHandle, audioBank);

	auto audioPlayerCtx = audioPlayer_.MakePlayerContext(audioBank);

	// visualizer
	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	audioPlayer_.musicVisualizer.DrawWaveform();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::BeginDisabled(!audioPlayerCtx);

	// player buttons
	DrawAudioPlayerButtons(ctx.uiTexturesConverter, audioPlayerCtx);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// elapsed time text
	DrawElapsedTimeText(audioPlayerCtx);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// volume slider
	DrawVolumeSlider(audioPlayerCtx);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// spatial sliders
	DrawSpatialDataSliders(audioPlayerCtx);

	ImGui::EndDisabled();

	if (audioPlayerCtx) { audioPlayerCtx->Commit(); }
}

void AssetPreviewViewerChild::DrawFontAssetPreview(ResourceContext& ctx)
{
	auto it = ctx.fontSelection.handleToGuiFont.find(ctx.fontSelection.fontHandle);
	assert(it != ctx.fontSelection.handleToGuiFont.end());

	auto* font = it->second;
	assert(font);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(fontWriterDisplay_.text.c_str()).x) * 0.5f);

	ImGui::PushFont(font);
	ImGui::PushStyleColor(ImGuiCol_Text, fontWriterDisplay_.color);

	ImGui::TextUnformatted(fontWriterDisplay_.text.c_str());

	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(
		(ImGui::GetContentRegionAvail().x - kPreviewFontTextInputSize) * 0.5f,
		0.0f));

	ImGui::SetNextItemWidth(kPreviewFontTextInputSize);

	ImGui::InputText("##previewText", &fontWriterDisplay_.text);

	ImGui::SameLine();

	if (ImGui::ColorButton("#textClrBtn", fontWriterDisplay_.color))
	{
		ImGui::OpenPopup("ColorPicker");
	}
	if (ImGui::BeginPopup("ColorPicker"))
	{
		ImGui::ColorPicker4("##textClrPicker", &fontWriterDisplay_.color.x);

		ImGui::EndPopup();
	}
}


void AssetPreviewViewerChild::DrawSpriteAssetSinglePreview(const Sprite& sprite, 
														   const GuiTextureConverter& loadTargetConverter,
														   const SpriteAtlas& spriteAtlas)
{
	auto tx = loadTargetConverter.FromSprite(sprite);
	assert(tx.textureId != 0);
	
	const float txScale = std::min(
		kPreviewTextureSize / tx.size.x,
		kPreviewTextureSize / tx.size.y
	);

	tx.size.x *= txScale;
	tx.size.y *= txScale;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - tx.size.x) * 0.5f);

	const auto checkboardPos = ImGui::GetCursorPos();

	DrawCheckerboard("##spriteCheckboard", ImVec4(0.0f, 0.0f, 0.0f, 0.6f), 0, tx.size, 16.0f);

	ImGui::SetCursorPos(checkboardPos);

	GuiImage(tx);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	const auto info = spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName,
												&SpriteInfo::filepath>(sprite);
	assert(info.has_value());

	const auto& [spriteName, filepath] = *info;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(spriteName.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(spriteName.c_str());

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(filepath.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(filepath.c_str());

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	const auto dimensionsStr = std::format("{} x {}", sprite.plot.rect.w, sprite.plot.rect.h);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(dimensionsStr.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(dimensionsStr.c_str());
}

void AssetPreviewViewerChild::DrawSpriteSeriesAssetsPreview(std::vector<Sprite>& sprites, 
															std::string_view seriesName,
														    const GuiTextureConverter& loadTargetConverter,
														    const GuiTextureConverter& uiTexturesConverter, 
															SpriteAtlas& spriteAtlas, float dt)
{
	if (sprites.empty())
	{
		return;
	}
	assert(!seriesName.empty());

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// Update animator
	auto& animator = spriteSeriesAnimator_;

	animator.index.max = sprites.size() - 1;
	animator.index.current = std::min(animator.index.current, animator.index.max);

	if (animator.state & PlayerState::Playing)
	{
		animator.elapsed += dt;
		if (animator.elapsed >= animator.animationSpeed)
		{
			++animator.index;
			animator.elapsed = 0.0f;
		}
	}

	assert(animator.index.current < sprites.size());
	
	// Main display sprite
	auto tx = loadTargetConverter.FromSprite(sprites[animator.index.current]);
	assert(tx.textureId != 0);

	const float txScale = std::min(
		kPreviewTextureSize / tx.size.x,
		kPreviewTextureSize / tx.size.y
	);

	tx.size.x *= txScale;
	tx.size.y *= txScale;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - tx.size.x) * 0.5f);

	const auto checkboardPos = ImGui::GetCursorPos();

	DrawCheckerboard("##spriteCheckboard", ImVec4(0.0f, 0.0f, 0.0f, 0.6f), 0, tx.size, 16.0f);

	ImGui::SetCursorPos(checkboardPos);

	GuiImage(tx);

	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	// Reorder series layout
	DrawSpriteSeriesReorderLayout(sprites, seriesName, loadTargetConverter, spriteAtlas);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// Index slider
	float indexSliderPos = std::max(
		(ImGui::GetContentRegionAvail().x - kPreviewSeriesAnimationIndexSliderSize) * 0.5f,
		0.0f);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indexSliderPos);
	ImGui::SetNextItemWidth(kPreviewSeriesAnimationIndexSliderSize);

	int idxDrag = static_cast<int>(animator.index.current);
	if (ImGui::DragInt("##index", &idxDrag, 1.0f, 0, static_cast<int>(animator.index.max)))
	{
		animator.index.current = static_cast<size_t>(idxDrag);
	}

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// player buttons
	DrawSpriteSeriesPlayerButtons(uiTexturesConverter);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// speed slider
	const float speedTextWidth = ImGui::CalcTextSize("speed:").x;

	const float speedSliderPos = std::max(
		(ImGui::GetContentRegionAvail().x - kPreviewSeriesAnimationSpeedSliderSize) * 0.5f,
		0.0f);

	const float speedTextStartX = speedSliderPos - ImGui::GetStyle().ItemSpacing.x - speedTextWidth;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + speedTextStartX);

	ImGui::TextUnformatted("speed:");

	ImGui::SameLine();

	ImGui::SetNextItemWidth(kPreviewSeriesAnimationSpeedSliderSize);
	
	animator.animationSpeed = std::max(animator.animationSpeed, 0.001f);

	ImGui::DragFloat("##speed", &animator.animationSpeed, 0.001f, 0.001f);
}

void AssetPreviewViewerChild::DrawSpriteSeriesPlayerButtons(const GuiTextureConverter& uiTexturesConverter)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	auto& animator = spriteSeriesAnimator_;

	const float spacing = ImGui::GetStyle().ItemSpacing.x;

	const float totalLayoutWidth = kPlayerButtonsTotalWidth + (3.0f * spacing);

	const float centerOffset = std::max(
		(ImGui::GetContentRegionAvail().x - totalLayoutWidth) * 0.5f,
		0.0f);

	const float startX = centerOffset;

	ImGui::SetCursorPosX(startX);

	auto rewindTx = uiTexturesConverter.FromSprite(playerIcons_.rewindSprite);
	assert(rewindTx.textureId != 0);

	if (GuiImageButton("rewindBtn", rewindTx))
	{
		--animator.index;
	}

	ImGui::SameLine(0.0f, spacing);

	const auto& playPauseSprite = (animator.state & PlayerState::Playing)
		? playerIcons_.pauseSprite
		: playerIcons_.playSprite;

	auto playPauseTx = uiTexturesConverter.FromSprite(playPauseSprite);
	assert(playPauseTx.textureId != 0);

	if (GuiImageButton("playPauseBtn", playPauseTx))
	{
		animator.state ^= PlayerState::Playing;
	}

	ImGui::SameLine(0.0f, spacing);

	auto fastForwardTx = uiTexturesConverter.FromSprite(playerIcons_.fastForwardSprite);
	assert(fastForwardTx.textureId != 0);

	if (GuiImageButton("fastForwardBtn", fastForwardTx))
	{
		++animator.index;
	}

	ImGui::PopStyleColor();
}

void AssetPreviewViewerChild::DrawAudioPlayerButtons(const GuiTextureConverter& uiTexturesConverter,
													 std::optional<AudioPlayerContext>& audioPlayerCtx)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	const float spacing = ImGui::GetStyle().ItemSpacing.x;

	const float totalLayoutWidth = kPlayerButtonsTotalWidth + (3.0f * spacing);

	const float centerOffset = std::max(
		(ImGui::GetContentRegionAvail().x - totalLayoutWidth) * 0.5f,
		0.0f);

	const float startX = centerOffset;

	ImGui::SetCursorPosX(startX);

	// Rewind button
	auto rewindTx = uiTexturesConverter.FromSprite(playerIcons_.rewindSprite);
	assert(rewindTx.textureId != 0);

	if (GuiImageButton("rewindBtn", rewindTx))
	{
		if (audioPlayerCtx) { audioPlayerCtx->RestartTrack(); }
	}
	else if (ImGui::IsItemActive())
	{
		if (audioPlayerCtx && audioPlayerCtx->audioType == AudioType::Music) 
		{ 
			audioPlayerCtx->RewindTrack();
		}
	}

	ImGui::SameLine(0.0f, spacing);

	const auto& playPauseSprite = (audioPlayer_.state & PlayerState::Playing)
		? playerIcons_.pauseSprite
		: playerIcons_.playSprite;

	auto playPauseTx = uiTexturesConverter.FromSprite(playPauseSprite);
	assert(playPauseTx.textureId != 0);

	if (GuiImageButton("playPauseBtn", playPauseTx))
	{
		audioPlayer_.state ^= PlayerState::Playing;
	}

	ImGui::SameLine(0.0f, spacing);

	auto fastForwardTx = uiTexturesConverter.FromSprite(playerIcons_.fastForwardSprite);
	assert(fastForwardTx.textureId != 0);

	GuiImageButton("fastForwardBtn", fastForwardTx);
	if (ImGui::IsItemActive())
	{
		if (audioPlayerCtx) { audioPlayerCtx->FastForwardTrack(); }
	}

	ImGui::PopStyleColor();
}

void AssetPreviewViewerChild::DrawSpriteSeriesReorderLayout(std::vector<Sprite>& sprites,
															std::string_view seriesName,
															const GuiTextureConverter& loadTargetConverter, 
															SpriteAtlas& spriteAtlas)
{
	assert(!sprites.empty());

	const float totalWidth = kPreviewSpriteSeriesReorderLayoutThumbnailSize * sprites.size();

	const float startPos = std::max(
		(ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f,
		0.0f);

	ImGui::SetCursorPosX(startPos);

	size_t removeIdx = std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < sprites.size(); ++i)
	{
		if (i > 0)
		{
			ImGui::SameLine(0.0f, 0.0f);
		}

		ImGui::PushID(static_cast<int>(i));

		auto spriteTx = loadTargetConverter.FromSprite(sprites[i]);
		assert(spriteTx.textureId != 0);

		const float txScale = std::min(
			kPreviewSpriteSeriesReorderLayoutThumbnailSize / spriteTx.size.x,
			kPreviewSpriteSeriesReorderLayoutThumbnailSize / spriteTx.size.y
		);

		spriteTx.size.x *= txScale;
		spriteTx.size.y *= txScale;

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		if (GuiImageButton("spriteReorder", spriteTx))
		{
			spriteSeriesAnimator_.index.current = i;
		}

		ImGui::PopStyleColor();

		if (spriteSeriesAnimator_.index.current == i)
		{
			ImGui::GetWindowDrawList()->AddRect(
				ImGui::GetItemRectMin(),
				ImGui::GetItemRectMax(),
				IM_COL32(255, 255, 255, 150),
				0.0f,
				0,
				1.0f
			);
		}

		if (ImGui::BeginPopupContextItem("spriteReorderPopupContext"))
		{
			if (ImGui::MenuItem("Remove"))
			{
				removeIdx = i;
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("ANIMATION_FRAME", &i, sizeof(i));

			ImGui::Text("Frame %d", i + 1);

			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATION_FRAME"))
			{
				const size_t sourceIndex = *static_cast<const size_t*>(payload->Data);
				const size_t targetIndex = i;

				if (sourceIndex != targetIndex)
				{
					assert(spriteAtlas.HasSpriteSeries(seriesName));

					std::swap(sprites[sourceIndex], sprites[targetIndex]);

					spriteAtlas.DefineSpriteSeries(seriesName, sprites);
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::PopID();
	}

	if (removeIdx < sprites.size())
	{
		core::Erase(sprites, sprites[removeIdx]);

		spriteSeriesAnimator_.index.max = sprites.size();
		spriteSeriesAnimator_.index.current = spriteSeriesAnimator_.index.current &
											  spriteSeriesAnimator_.index.max;

		spriteAtlas.DefineSpriteSeries(seriesName, sprites);
	}
}

Result<Void> AssetPreviewViewerChild::LoadResources(SceneFixture& fixture)
{
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Auxiliary texture repository was null");
	}

	TRY(ResourcePath::Sprite("ui/editor/asset_player_play_icon_large.png"),
		playIconPath);
	TRY(ResourcePath::Sprite("ui/editor/asset_player_pause_icon_large.png"),
		pauseIconPath);
	TRY(ResourcePath::Sprite("ui/editor/asset_player_rewind_icon_large.png"),
		rewindIconPath);
	TRY(ResourcePath::Sprite("ui/editor/asset_player_fast_forward_icon_large.png"),
		fastForwardIconPath);

	auto& atlas = auxRepo->GetSpriteAtlas();

	TRY_ASSIGN(playerIcons_.playSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(playIconPath) }));
	TRY_ASSIGN(playerIcons_.pauseSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(pauseIconPath) }));
	TRY_ASSIGN(playerIcons_.rewindSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(rewindIconPath) }));
	TRY_ASSIGN(playerIcons_.fastForwardSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(fastForwardIconPath) }));

	TRY(ResourcePath::Music("futuristic_beat.mp3"), futureBeatPath);
	TRY(ResourcePath::Music("one_more_time.ogg"), oneMoreTimePath);

	auto& audioBank = fixture.GetAudioBank();

	TRY(audioBank.LoadAudio({ .audioType = AudioType::Music, .filepath = std::move(futureBeatPath) }));
	TRY(audioBank.LoadAudio({ .audioType = AudioType::Sound, .filepath = std::move(oneMoreTimePath) }));

	return kVoid;
}


Result<Void> AssetPreviewViewerChild::Init(SceneFixture& fixture)
{
	TRY(LoadResources(fixture));

	auto audioPlayerE = ECS::CreateEntity();
	assert(audioPlayerE.IsValid());
	audioPlayerE.AddComponent<InspectorTag>();

	audioPlayer_.entityId = audioPlayerE.GetID();
	audioPlayer_.musicVisualizer.Start();

	return kVoid;
}

void AssetPreviewViewerChild::Reset()
{
	spriteSeriesAnimator_.Reset();
	audioPlayer_.Reset();
}

void AssetPreviewViewerChild::TearDown()
{
	audioPlayer_.musicVisualizer.Stop();
}

} // ui

#endif