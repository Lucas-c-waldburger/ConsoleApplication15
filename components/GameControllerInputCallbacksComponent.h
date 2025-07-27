#pragma once
#include "BaseComponent.h"
#include "../callbacks/EventCallback.h"
#include "../inputs/controller/GameControllerInputSource.h"

struct GameControllerInputCallbacks : public BaseComponent<GameControllerInputCallbacks>
{
	std::unordered_map<GameControllerInputSource, EventCallbackView> table;
};
