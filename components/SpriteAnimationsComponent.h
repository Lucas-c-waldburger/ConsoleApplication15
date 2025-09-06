#pragma once
#include "BaseComponent.h"
#include "../sprite/SpriteAnimationSeries.h"
#include "../core/commonObjects.h"
#include "../core/Dictionary.h"

struct SpriteAnimations : public BaseComponent<SpriteAnimations>
{
    UnorderedDictionary<SpriteAnimationSeries> table;
    std::string current;
};