#include "AssetPreviewViewerChild.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"

namespace ui {

namespace {

static constexpr float kPreviewTextureSize = 256.0f;
static constexpr float kPreviewSeriesAnimationIndexSliderSize = 300.0f;
static constexpr float kPreviewSeriesAnimationSpeedSliderSize = 100.0f;
static constexpr float kPlayerButtonSize = 64.0f;
static constexpr float kPlayerButtonPadding = 15.0f;
static constexpr float kPlayerButtonsTotalWidth = kPlayerButtonSize * 3.0f;
static constexpr float kAudioScrubAdvancePercent = 0.001f;
static constexpr float kPreviewMusicVisualizerProgressBarSize = 300.0f;
static constexpr float kPreviewAudioVolumeSiderSize = 100.0f;

struct AudioPlayerContext
{
	float curTrackPos = 0.0f;
	float maxTrackLen = 0.0f;
	float scrubSec = 0.0f;

	void RewindTrack(Entity& e) const
	{
		if (e.HasComponent<ActiveAudio>())
		{
			const auto& aa = e.GetComponent<ActiveAudio>();

			auto& req = e.AddComponent<AudioUpdateRequest>();
			req.instanceId = aa.instanceId;
			req.settings.trackPosition = std::max(curTrackPos - scrubSec, 0.0f);
		}
	}

	void FastForwardTrack(Entity& e) const
	{
		if (e.HasComponent<ActiveAudio>())
		{
			const auto& aa = e.GetComponent<ActiveAudio>();

			auto& req = e.AddComponent<AudioUpdateRequest>();
			req.instanceId = aa.instanceId;
			req.settings.trackPosition = std::min(curTrackPos + scrubSec, maxTrackLen);
		}
	}

	static AudioPlayerContext Create(Entity& e, const AudioBank& audioBank)
	{
		AudioPlayerContext ctx{};

		if (!e.HasComponent<ActiveAudio>())
		{
			return ctx;
		}

		const auto& aa = e.GetComponent<ActiveAudio>();

		ctx.curTrackPos = aa.settings.trackPosition;

		if (auto lenOp = audioBank.GetAudioInfo<&AudioInfo::length>(aa.audioHandle))
		{
			ctx.maxTrackLen = static_cast<float>(*lenOp) / 1000.0f;
		}

		ctx.scrubSec = ctx.maxTrackLen * kAudioScrubAdvancePercent;
		
		return ctx;
	}

	std::string MakeElapsedTimeText() const
	{
		int curMinutes = static_cast<int>(curTrackPos);
		int maxMinutes = static_cast<int>(maxTrackLen);

		const float curSecondsFractional = curTrackPos - static_cast<float>(curMinutes);
		const float maxSecondsFractional = maxTrackLen - static_cast<float>(maxMinutes);

		int curSeconds = static_cast<int>(std::round(curSecondsFractional * 60.0f));
		int maxSeconds = static_cast<int>(std::round(maxSecondsFractional * 60.0f));

		if (curSeconds == 60) { curMinutes += 1; curSeconds = 0; }
		if (maxSeconds == 60) { maxMinutes += 1; maxSeconds = 0; }

		return std::format("{}:{} / {}:{}", curMinutes, curSeconds, maxMinutes, maxSeconds);
	}
};

} // unnamed

void AssetPreviewViewerChild::AudioPlayer::Update(const Handle<Audio>& handle, 
												  const AudioBank& audioBank)
{
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
				if (aa.status == AudioStatus::Playing)
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

int AssetPreviewViewerChild::AudioPlayer::GetVolume() const
{
	if (auto e = ECS::GetEntityByID(entityId); e.HasComponent<ActiveAudio>())
	{
		return e.GetComponent<ActiveAudio>().settings.volume;
	}

	return 0;
}

void AssetPreviewViewerChild::AudioPlayer::SetVolume(int vol)
{
	if (auto e = ECS::GetEntityByID(entityId); e.HasComponent<ActiveAudio>())
	{
		e.AddComponent<AudioUpdateRequest>().settings.volume = vol;
	}
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
}


void AssetPreviewViewerChild::DrawSpriteAssetPreview(ResourceContext& ctx,
													 const SpriteAtlas& spriteAtlas, float dt)
{
	if (ctx.spriteSelection.SpriteSeriesSelectedAtTopLevel())
	{
		auto sprites = spriteAtlas.GetSpriteSeries(ctx.spriteSelection.spriteSeries);

		DrawSpriteSeriesAssetsPreview(sprites, 
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
	auto e = ECS::GetEntityByID(audioPlayer_.entityId);

	if (!audioBank.IsAudioValid(ctx.audioSelection.audioHandle))
	{
		audioPlayer_.Reset();

		return;
	}

	audioPlayer_.Update(ctx.audioSelection.audioHandle, audioBank);

	// visualizer
	audioPlayer_.musicVisualizer.DrawWaveform();

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// player buttons
	DrawAudioPlayerButtons(ctx.uiTexturesConverter, audioBank);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// volume slider
	const float volumeTextWidth = ImGui::CalcTextSize("volume:").x;

	const float volumeSliderPos = std::max(
		(ImGui::GetContentRegionAvail().x - kPreviewAudioVolumeSiderSize) * 0.5f,
		0.0f);

	const float volumeTextStartX = volumeSliderPos - ImGui::GetStyle().ItemSpacing.x - volumeTextWidth;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + volumeTextStartX);

	ImGui::TextUnformatted("volume:");

	ImGui::SameLine();

	ImGui::SetNextItemWidth(kPreviewAudioVolumeSiderSize);

	int volume = audioPlayer_.GetVolume();
	if (ImGui::DragInt("##volume", &volume, 1, 0, MIX_MAX_VOLUME))
	{
		audioPlayer_.SetVolume(volume);
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
	
	GuiImage(tx);

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

void AssetPreviewViewerChild::DrawSpriteSeriesAssetsPreview(const std::vector<Sprite>& sprites, 
														    const GuiTextureConverter& loadTargetConverter,
														    const GuiTextureConverter& uiTexturesConverter, 
															const SpriteAtlas& spriteAtlas, float dt)
{
	if (sprites.empty())
	{
		return;
	}

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

	GuiImage(tx);

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// index slider
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
													 const AudioBank& audioBank)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	const float spacing = ImGui::GetStyle().ItemSpacing.x;

	const float totalLayoutWidth = kPlayerButtonsTotalWidth + (3.0f * spacing);

	const float centerOffset = std::max(
		(ImGui::GetContentRegionAvail().x - totalLayoutWidth) * 0.5f,
		0.0f);

	const float startX = centerOffset;

	ImGui::SetCursorPosX(startX);

	auto e = ECS::GetEntityByID(audioPlayer_.entityId);

	const auto audioPlayerCtx = AudioPlayerContext::Create(e, audioBank);

	auto rewindTx = uiTexturesConverter.FromSprite(playerIcons_.rewindSprite);
	assert(rewindTx.textureId != 0);

	GuiImageButton("rewindBtn", rewindTx);
	if (ImGui::IsItemActive())
	{
		audioPlayerCtx.RewindTrack(e);
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
		audioPlayerCtx.FastForwardTrack(e);
	}

	ImGui::PopStyleColor();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// elapsed time text
	const auto timeText = audioPlayerCtx.MakeElapsedTimeText();

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(timeText.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(timeText.c_str());
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

void AssetPreviewViewerChild::TearDown()
{
	audioPlayer_.musicVisualizer.Stop();
}

} // ui

#endif