#include "AudioController.h"
#include <algorithm>

template <SomeMixType T>
int AudioController::GetVolume(T* audioPtr)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return Mix_GetMusicVolume(audioPtr);
	}
	else
	{
		return Mix_VolumeChunk(audioPtr, -1);
	}
}

template <SomeMixType T>
void AudioController::SetVolume(T* audioPtr, int volume)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		Mix_VolumeMusic(std::clamp(volume, 0, MIX_MAX_VOLUME));
	}
	else
	{
		Mix_VolumeChunk(audioPtr, std::clamp(volume, 0, MIX_MAX_VOLUME));
	}
}

template <SomeMixType T>
bool AudioController::IsPlaying(int channelIdx)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return Mix_PlayingMusic() != 0 && Mix_PausedMusic() == 0;
	}
	else
	{
		return Mix_Playing(channelIdx) != 0 && Mix_Paused(channelIdx) == 0;
	}
}

template <SomeMixType T>
bool AudioController::IsStopped(int channelIdx)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return Mix_PlayingMusic() == 0 && Mix_PausedMusic() == 0;
	}
	else
	{
		return Mix_Playing(channelIdx) == 0 && Mix_Paused(channelIdx) == 0;
	}
}

template <SomeMixType T>
bool AudioController::IsPaused(int channelIdx)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return Mix_PausedMusic() != 0;
	}
	else
	{
		return Mix_Paused(channelIdx) != 0;
	}
}

template <SomeMixType T>
bool AudioController::IsFadingIn(int channelIdx)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return (Mix_FadingMusic() == MIX_FADING_IN);
	}
	else
	{
		return (Mix_FadingChannel(channelIdx) == MIX_FADING_IN);
	}
}

template <SomeMixType T>
bool AudioController::IsFadingOut(int channelIdx)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return (Mix_FadingMusic() == MIX_FADING_OUT);
	}
	else
	{
		return (Mix_FadingChannel(channelIdx) == MIX_FADING_OUT);
	}
}

template <SomeMixType T>
AudioStatus AudioController::GetAudioStatus(int channelIdx)
{
	if (IsPaused<T>(channelIdx))
	{
		return AudioStatus::Paused;
	}
	if (IsFadingOut<T>(channelIdx))
	{
		return AudioStatus::Stopping;
	}
	if (IsPlaying<T>(channelIdx))
	{
		return AudioStatus::Playing;
	}

	return AudioStatus::Stopped;
}

template <SomeMixType T>
void AudioController::Pause(int channelIdx)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		Mix_PauseMusic();
	}
	else
	{
		Mix_Pause(channelIdx);
	}
}

template <SomeMixType T>
void AudioController::Resume(int channelIdx)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		Mix_ResumeMusic();
	}
	else
	{
		Mix_Resume(channelIdx);
	}
}

template <SomeMixType T>
int AudioController::Play(T* audioPtr, int channelIdx, int loops, int fadeInMs)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		if (fadeInMs > 0)
		{
			return Mix_FadeInMusic(audioPtr, loops, fadeInMs);
		}
		else
		{
			return Mix_PlayMusic(audioPtr, loops);
		}
	}
	else
	{
		if (fadeInMs > 0)
		{
			return Mix_FadeInChannel(channelIdx, audioPtr, loops, fadeInMs);
		}
		else
		{
			return Mix_PlayChannel(channelIdx, audioPtr, loops);
		}
	}
}

template <SomeMixType T>
int AudioController::Stop(int channelIdx, int fadeOutMs)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		if (fadeOutMs > 0)
		{
			return Mix_FadeOutMusic(fadeOutMs);
		}
		else
		{
			return Mix_HaltMusic();
		}
	}
	else
	{
		if (fadeOutMs > 0)
		{
			return Mix_FadeOutChannel(channelIdx, fadeOutMs);
		}
		else
		{
			return Mix_HaltChannel(channelIdx);
		}
	}
}

template <SomeMixType T>
int AudioController::FadeIn(T* audioPtr, int channelIdx, int fadeInMs, int loops)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return Mix_FadeInMusic(audioPtr, loops, fadeInMs);
	}
	else
	{
		return Mix_FadeInChannel(channelIdx, audioPtr, loops, fadeInMs);
	}
}

template <SomeMixType T>
int AudioController::FadeOut(int channelIdx, int fadeOutMs)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return Mix_FadeOutMusic(fadeOutMs);
	}
	else
	{
		return Mix_FadeOutChannel(channelIdx, fadeOutMs);
	}
}

template <SomeMixType T>
double AudioController::GetTrackPositionSec(T* audioPtr)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		return Mix_GetMusicPosition(audioPtr);
	}
	else
	{
		return -1.0;
	}
}

int AudioController::SetTrackPositionSec(double sec)
{
	return Mix_SetMusicPosition(sec);
}

int AudioController::SetPanning(int channelIdx, uint8_t left, uint8_t right)
{
	return Mix_SetPanning(channelIdx, left, right);
}

int AudioController::SetDistance(int channelIdx, uint8_t distance)
{
	return Mix_SetDistance(channelIdx, distance);
}

int AudioController::SetSpatialPosition(int channelIdx, int16_t angle, uint8_t distance)
{
	return Mix_SetPosition(channelIdx, angle, distance);
}

template <SomeMixType T>
int AudioController::Restart(T* audioPtr, int channelIdx, int loops, int fadeInMs)
{
	if constexpr (std::same_as<T, Mix_Music>)
	{
		Mix_RewindMusic();

		return 0;
	}
	else
	{
		Mix_HaltChannel(channelIdx);

		if (fadeInMs > 0)
		{
			return Mix_FadeInChannel(channelIdx, audioPtr, loops, fadeInMs);
		}
		else
		{
			return Mix_PlayChannel(channelIdx, audioPtr, loops);
		}
	}
}

template int AudioController::GetVolume(Mix_Music*);
template int AudioController::GetVolume(Mix_Chunk*);

template void AudioController::SetVolume(Mix_Music*, int);
template void AudioController::SetVolume(Mix_Chunk*, int);

template bool AudioController::IsPlaying<Mix_Music>(int);
template bool AudioController::IsPlaying<Mix_Chunk>(int);

template bool AudioController::IsStopped<Mix_Music>(int);
template bool AudioController::IsStopped<Mix_Chunk>(int);

template bool AudioController::IsPaused<Mix_Music>(int);
template bool AudioController::IsPaused<Mix_Chunk>(int);

template bool AudioController::IsFadingIn<Mix_Music>(int);
template bool AudioController::IsFadingIn<Mix_Chunk>(int);

template bool AudioController::IsFadingOut<Mix_Music>(int);
template bool AudioController::IsFadingOut<Mix_Chunk>(int);

template AudioStatus AudioController::GetAudioStatus<Mix_Music>(int);
template AudioStatus AudioController::GetAudioStatus<Mix_Chunk>(int);

template void AudioController::Pause<Mix_Music>(int);
template void AudioController::Pause<Mix_Chunk>(int);

template void AudioController::Resume<Mix_Music>(int);
template void AudioController::Resume<Mix_Chunk>(int);

template int AudioController::Play(Mix_Music*, int, int, int);
template int AudioController::Play(Mix_Chunk*, int, int, int);

template int AudioController::Stop<Mix_Music>(int, int);
template int AudioController::Stop<Mix_Chunk>(int, int);

template int AudioController::FadeIn(Mix_Music*, int, int, int);
template int AudioController::FadeIn(Mix_Chunk*, int, int, int);

template int AudioController::FadeOut<Mix_Music>(int, int);
template int AudioController::FadeOut<Mix_Chunk>(int, int);

template double AudioController::GetTrackPositionSec(Mix_Music*);
template double AudioController::GetTrackPositionSec(Mix_Chunk*);

template int AudioController::Restart(Mix_Music*, int, int, int);
template int AudioController::Restart(Mix_Chunk*, int, int, int);