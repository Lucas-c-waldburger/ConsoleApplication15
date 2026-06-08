#pragma once
#include "AudioEnums.h"
#include "AudioPtrs.h"
#include "../core/commonObjects.h"
#include <SDL_mixer.h>
#include <memory>
#include <string>

template <typename T>
concept SomeMixType = std::same_as<T, Mix_Chunk> ||
                      std::same_as<T, Mix_Music>;


struct AudioSpatialData
{
    using Panning = HandedPair<uint8_t>;

    std::optional<int16_t> angle;
    std::optional<uint8_t> distance;
    std::optional<Panning> panning;

    friend constexpr bool operator==(const AudioSpatialData& lhs, 
                                     const AudioSpatialData& rhs)
    {
        return lhs.angle == rhs.angle && lhs.distance == rhs.distance &&
               lhs.panning == rhs.panning;
    }
}; 