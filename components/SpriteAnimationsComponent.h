#pragma once
#include "BaseComponent.h"
#include "../sprite/SpriteAnimationSeriesMap.h"
#include "../core/commonObjects.h"

struct SpriteAnimations : public BaseComponent<SpriteAnimations>
{
    //SpriteAnimationsTable table;

    std::unordered_map<HashName, SpriteAnimationSeries> table;
    HashName current;
};