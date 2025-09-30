#pragma once
#include <SDL_mixer.h>

struct AudioSettings
{
    struct FadeMs
    {
        int in = 0;
        int out = 0;
        friend constexpr bool operator==(const FadeMs& lhs, const FadeMs& rhs)
        {
            return lhs.in == rhs.in && lhs.out == rhs.out;
        }
    };

    struct Spatialized
    {
        bool enabled = false;
        float maxDistance = 0.0f;
        friend constexpr bool operator==(const Spatialized& lhs, const Spatialized& rhs)
        {
            return lhs.enabled == rhs.enabled && lhs.maxDistance == rhs.maxDistance;
        }
    };

    int volume = MIX_MAX_VOLUME / 2;
    int loopCount = 0;
    FadeMs fadeMs = {};
    Spatialized spatialized = {};

    friend constexpr bool operator==(const AudioSettings& lhs, const AudioSettings& rhs)
    {
        return lhs.volume == rhs.volume && lhs.loopCount == rhs.loopCount &&
            lhs.fadeMs == rhs.fadeMs && lhs.spatialized == rhs.spatialized;
    }
};