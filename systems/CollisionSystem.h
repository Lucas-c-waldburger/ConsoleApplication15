#pragma once
#include "../core/Result.h"
#include "../core/commonObjects.h"
#include "../events/EventBus2.h"

class B2World;

Result<Void> DispatchCollisionEvents(const B2World* world, EventBus& bus);

