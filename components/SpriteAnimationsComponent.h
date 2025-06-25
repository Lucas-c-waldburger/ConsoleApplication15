#pragma once
#include "BaseComponent.h"
#include "../sprite/SpriteAnimationSeriesMap.h"

//struct SpriteAnimations : public BaseComponent<SpriteAnimations, 13>
struct SpriteAnimations : public BaseComponent<SpriteAnimations>
{
    SpriteAnimationSeriesMap map;
    bool dirty = true;
};