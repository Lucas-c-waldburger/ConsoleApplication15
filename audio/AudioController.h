#pragma once
#include "AudioCommon.h"

class AudioController
{
public:
	template <SomeMixType T>
	static int GetVolume(T* audioPtr);

	template <SomeMixType T>
	static void SetVolume(T* audioPtr, int volume);

	template <SomeMixType T>
	static bool IsPlaying(int channelIdx);

	template <SomeMixType T>
	static bool IsStopped(int channelIdx);

	template <SomeMixType T>
	static bool IsPaused(int channelIdx);

	template <SomeMixType T>
	static bool IsFadingIn(int channelIdx);

	template <SomeMixType T>
	static bool IsFadingOut(int channelIdx);

	template <SomeMixType T>
	static AudioStatus GetAudioStatus(int channelIdx);

	template <SomeMixType T>
	static void Pause(int channelIdx);

	template <SomeMixType T>
	static void Resume(int channelIdx);

	template <SomeMixType T>
	static int Play(T* audioPtr, int channelIdx, int loops, int fadeInMs);

	template <SomeMixType T>
	static int Stop(int channelIdx, int fadeOutMs);

	template <SomeMixType T>
	static int FadeIn(T* audioPtr, int channelIdx, int fadeInMs, int loops);

	template <SomeMixType T>
	static int FadeOut(int channelIdx, int fadeOutMs);

	template <SomeMixType T>
	static double GetTrackPositionSec(T* audioPtr);

	static int SetTrackPositionSec(double sec);

	static int SetPanning(int channelIdx, uint8_t left, uint8_t right);

	static int SetDistance(int channelIdx, uint8_t distance);

	static int SetSpatialPosition(int channelIdx, int16_t angle, uint8_t distance);

	template <SomeMixType T>
	static int Restart(T* audioPtr, int channelIdx, int loops, int fadeInMs);

private:
	AudioController() = default;
};

extern template int AudioController::GetVolume(Mix_Music*);
extern template int AudioController::GetVolume(Mix_Chunk*);

extern template void AudioController::SetVolume(Mix_Music*, int);
extern template void AudioController::SetVolume(Mix_Chunk*, int);

extern template bool AudioController::IsPlaying<Mix_Music>(int);
extern template bool AudioController::IsPlaying<Mix_Chunk>(int);

extern template bool AudioController::IsStopped<Mix_Music>(int);
extern template bool AudioController::IsStopped<Mix_Chunk>(int);

extern template bool AudioController::IsPaused<Mix_Music>(int);
extern template bool AudioController::IsPaused<Mix_Chunk>(int);

extern template bool AudioController::IsFadingIn<Mix_Music>(int);
extern template bool AudioController::IsFadingIn<Mix_Chunk>(int);

extern template bool AudioController::IsFadingOut<Mix_Music>(int);
extern template bool AudioController::IsFadingOut<Mix_Chunk>(int);

extern template AudioStatus AudioController::GetAudioStatus<Mix_Music>(int);
extern template AudioStatus AudioController::GetAudioStatus<Mix_Chunk>(int);

extern template void AudioController::Pause<Mix_Music>(int);
extern template void AudioController::Pause<Mix_Chunk>(int);

extern template void AudioController::Resume<Mix_Music>(int);
extern template void AudioController::Resume<Mix_Chunk>(int);

extern template int AudioController::Play(Mix_Music*, int, int, int);
extern template int AudioController::Play(Mix_Chunk*, int, int, int);

extern template int AudioController::Stop<Mix_Music>(int, int);
extern template int AudioController::Stop<Mix_Chunk>(int, int);

extern template int AudioController::FadeIn(Mix_Music*, int, int, int);
extern template int AudioController::FadeIn(Mix_Chunk*, int, int, int);

extern template int AudioController::FadeOut<Mix_Music>(int, int);
extern template int AudioController::FadeOut<Mix_Chunk>(int, int);

extern template double AudioController::GetTrackPositionSec(Mix_Music*);
extern template double AudioController::GetTrackPositionSec(Mix_Chunk*);

extern template int AudioController::Restart(Mix_Music*, int, int, int);
extern template int AudioController::Restart(Mix_Chunk*, int, int, int);