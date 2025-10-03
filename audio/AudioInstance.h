#pragma once
#include <SDL_mixer.h>
#include "../core/Handle.h"
#include "AudioCommon.h"

struct AudioInstance;
using AudioInstanceID = Handle<AudioInstance>;

template <typename T>
struct AudioInstanceResource
{
    T* audioPtr = nullptr;
    AudioInstanceID id;
};

using SoundInstanceResource = AudioInstanceResource<Mix_Chunk>;
using MusicInstanceResource = AudioInstanceResource<Mix_Music>;