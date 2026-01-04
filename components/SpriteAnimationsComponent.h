#pragma once
#include "ComponentConcepts.h"
#include "../sprite/SpriteAnimationSeries.h"
#include "../core/commonObjects.h"
#include "../core/Dictionary.h"

struct SpriteAnimations : public BaseComponent<SpriteAnimations>
{
    UnorderedDictionary<SpriteAnimationSeries> table;
    std::string current;
};

struct NeedsAnimationUpdate : BaseComponent<NeedsAnimationUpdate> {};

struct SpriteAnimationComponent : BaseComponent<SpriteAnimationComponent>,
                                  TriggersUpdate<NeedsAnimationUpdate>
{
    Handle<TextureAtlas> sourceAtlas;
    std::string spriteSeriesName;
    size_t currentIndex = 0;
};

