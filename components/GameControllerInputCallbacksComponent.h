#pragma once
#include "BaseComponent.h"
#include "../callbacks/EventCallbackHandle.h"
#include "../inputs/controller/GameControllerInputSource.h"

struct GameControllerInputCallbacks : public BaseComponent<GameControllerInputCallbacks>
{
	std::unordered_map<GameControllerInputSource, std::vector<Handle<EventCallback>>> table;
};
