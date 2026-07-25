#pragma once
#include <vector>
#include "System.h"
#include "../events/data/EntityActions.h"
#include "../core/Signal.h"
#include "../ecs/Ecs.h"

class EventBus;

class EntityDestructionSystem : public System
{
public:
	void Update(EventBus& bus);

private:
	//std::vector<events::EntityDestroyed> destroyEvents_;
};