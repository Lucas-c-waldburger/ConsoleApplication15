#pragma once
#include <vector>
#include "System.h"
#include "../events/data/EntityActions.h"

class EventBus2;

class EntityDestructionSystem : public System
{
public:
	void Update(EventBus2& bus);

private:
	//std::vector<events::EntityDestroyed> destroyEvents_;
};