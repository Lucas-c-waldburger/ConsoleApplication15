#include "AudioPlayerContext.h"

#if IMGUI_ENABLED
#include "../../audio/AudioBank.h"

namespace ui {

namespace {
	
static constexpr float kAudioScrubAdvancePercent = 0.001f;

bool CompareAudioChannelAndUpdateSettings(const AudioChannelSettings& a, const AudioUpdateSettings& b)
{
	static constexpr auto comp = [](const auto& setA, const auto& setB) {
		return !setB.has_value() || (setA == setB);
	};

	return comp(a.volume, b.volume) && comp(a.fadeMs, b.fadeMs) &&
		   comp(a.loopCount, b.loopCount) && comp(a.trackPosition, b.trackPosition) &&
		   comp(a.spatial.angle, b.spatial.angle) &&
		   comp(a.spatial.distance, b.spatial.distance) &&
		   comp(a.spatial.panning, b.spatial.panning);
}

} // unnamed

void AudioPlayerContext::RestartTrack()
{
	updateRequest.command = AudioPlayCommand::Restart;
}

void AudioPlayerContext::RewindTrack()
{
	updateRequest.settings.trackPosition = std::max(curTrackPos - scrubSec, 0.0f);
}

void AudioPlayerContext::FastForwardTrack()
{
	updateRequest.settings.trackPosition = std::min(curTrackPos + scrubSec, maxTrackLen);
}

std::optional<AudioPlayerContext> AudioPlayerContext::Create(Entity& e, const AudioBank& audioBank)
{
	if (!e.HasComponent<ActiveAudio>())
	{
		return std::nullopt;
	}

	const auto& aa = e.GetComponent<ActiveAudio>();

	AudioPlayerContext ctx{};

	ctx.curTrackPos = aa.settings.trackPosition;

	auto info = audioBank.GetAudioInfo<&AudioInfo::audioType,
		&AudioInfo::length>(aa.audioHandle);
	if (info)
	{
		ctx.audioType = std::get<0>(*info);
		ctx.maxTrackLen = static_cast<float>(std::get<1>(*info)) / 1000.0f;
	}

	ctx.scrubSec = ctx.maxTrackLen * kAudioScrubAdvancePercent;

	ctx.updateRequest.instanceId = aa.instanceId;

	ctx.entity = e;

	return ctx;
}

void AudioPlayerContext::Commit()
{
	assert(entity.HasComponent<ActiveAudio>());

	auto& aa = entity.GetComponent<ActiveAudio>();

	if (updateRequest.command == AudioPlayCommand::None &&
		updateRequest.settings == Null<AudioUpdateSettings>())
	{
		return;
	}

	assert(!entity.HasComponent<AudioUpdateRequest>());

	entity.AddComponent(std::move(updateRequest));
}

std::string AudioPlayerContext::MakeElapsedTimeText() const
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

int AudioPlayerContext::GetVolume() const
{
	assert(entity.HasComponent<ActiveAudio>());

	return entity.GetComponent<ActiveAudio>().settings.volume;
}
int AudioPlayerContext::GetLoopCount() const
{
	assert(entity.HasComponent<ActiveAudio>());

	return entity.GetComponent<ActiveAudio>().settings.loopCount;
}
AudioFadeMs AudioPlayerContext::GetFadeMs() const
{
	assert(entity.HasComponent<ActiveAudio>());

	return entity.GetComponent<ActiveAudio>().settings.fadeMs;
}
float AudioPlayerContext::GetTrackPos() const
{
	assert(entity.HasComponent<ActiveAudio>());

	return entity.GetComponent<ActiveAudio>().settings.trackPosition;
}
AudioPlayerContext::AudioSpatialDrawData AudioPlayerContext::GetSpatialData() const
{
	assert(entity.HasComponent<ActiveAudio>());

	const auto& aaSpatial = entity.GetComponent<ActiveAudio>().settings.spatial;

	AudioSpatialDrawData spatial{};
	if (aaSpatial.angle) { spatial.angle = static_cast<int>(*aaSpatial.angle); }
	if (aaSpatial.distance) { spatial.distance = static_cast<int>(*aaSpatial.distance); }
	if (aaSpatial.panning)
	{
		spatial.panning = 128 - static_cast<int>(aaSpatial.panning->left);
	}

	return spatial;
}



} // ui

#endif