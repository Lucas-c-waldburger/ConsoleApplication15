#pragma once
#include "BaseComponent.h"
#include "../sprite/SpriteAnimationSeries.h"
#include "../core/commonObjects.h"

struct SpriteAnimations : public BaseComponent<SpriteAnimations>
{
    std::unordered_map<HashName, SpriteAnimationSeries> table;
    HashName current;
};