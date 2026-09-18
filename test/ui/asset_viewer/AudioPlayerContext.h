#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"

class AudioBank;

namespace ui {

struct AudioPlayerContext
{
	struct AudioSpatialDrawData
	{
		int angle = 0;
		int distance = 0;
		int panning = 0;
	};

	void RestartTrack();
	void RewindTrack();
	void FastForwardTrack();

	int GetVolume() const;
	int GetLoopCount() const;
	AudioFadeMs GetFadeMs() const;
	float GetTrackPos() const;
	AudioSpatialDrawData GetSpatialData() const;

	std::string MakeElapsedTimeText() const;

	void Commit();

	static std::optional<AudioPlayerContext> Create(Entity& e, const AudioBank& audioBank);

	Entity entity;
	float curTrackPos = 0.0f;
	float maxTrackLen = 0.0f;
	float scrubSec = 0.0f;
	AudioUpdateRequest updateRequest;
	AudioType audioType;
};


} // ui

#endif