#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include "../../ecs/EntityT.h"
#include "../../core/commonObjects.h"
 
namespace events {

struct SpriteIndexChange : IEventData<SpriteIndexChange>
{
	Entity_t entity = kInvalidEntity;
	HashName seriesName;
	DataRecord<size_t> index = { 0, 0 };
};

struct SpriteSeriesChange : IEventData<SpriteSeriesChange>
{ 
	Entity_t entity = kInvalidEntity;
	DataRecord<HashName> seriesName;
};

// GROUP
using SpriteAnimationEventGroup = EventGroup<
	SpriteIndexChange,
	SpriteSeriesChange
>;

}