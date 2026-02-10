#pragma once
#include "ComponentConcepts.h"
#include "../core/commonObjects.h"
#include "../core/Dictionary.h"

struct NeedsAnimationUpdate : BaseComponent<NeedsAnimationUpdate> {};

struct SpriteAnimationComponent : BaseComponent<SpriteAnimationComponent>,
                                  TriggersUpdate<NeedsAnimationUpdate>
{
    std::string spriteSeriesName;
    size_t currentIndex = 0;
};

