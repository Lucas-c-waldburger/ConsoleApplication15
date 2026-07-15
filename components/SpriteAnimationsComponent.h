#pragma once
#include "ComponentConcepts.h"
#include "../core/commonObjects.h"
#include "../core/Dictionary.h"

struct NeedsAnimationUpdate : BaseComponent<NeedsAnimationUpdate> {};

struct SpriteSeriesIndex
{
    size_t current = 0;
    size_t max = 0;

    friend constexpr size_t& operator++(SpriteSeriesIndex& idx) noexcept
    {
        idx.current = (idx.current + 1 <= idx.max) ? idx.current + 1 : 0;
        return idx.current;
    }
    friend constexpr size_t& operator--(SpriteSeriesIndex& idx) noexcept
    {
        idx.current = (idx.current == 0) ? idx.max : idx.current - 1;
        return idx.current;
    }
    friend constexpr bool operator==(const SpriteSeriesIndex& lhs, 
                                     const SpriteSeriesIndex& rhs) noexcept
    {
        return lhs.current == rhs.current && lhs.max == rhs.max;
    };
};

struct SpriteAnimationComponent : BaseComponent<SpriteAnimationComponent>,
                                  TriggersUpdate<NeedsAnimationUpdate>
{
    std::string spriteSeriesName;
    SpriteSeriesIndex index;

    friend bool operator==(const SpriteAnimationComponent& lhs, 
                           const SpriteAnimationComponent& rhs) noexcept
    {
        return lhs.spriteSeriesName == rhs.spriteSeriesName &&
               lhs.index == rhs.index;
    }
};

