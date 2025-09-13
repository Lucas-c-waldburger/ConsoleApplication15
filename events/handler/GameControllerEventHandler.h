#pragma once
#include "../../inputs/controller/GameController.h"
#include "../../inputs/controller/GameControllerInputUpdater.h"
#include "../EventBus2.h"
#include <unordered_map>
#include <set>

class GameControllerEventHandler
{
public:
	GameControllerEventHandler() = default;
	~GameControllerEventHandler();

	void HandleDeviceEvent(const SDL_Event& ev, EventBus2& bus);
	void HandleInputEvent(const SDL_Event& ev);
	void Finalize(EventBus2& bus);

private:
	void UpdateControllerStateComponents();

	std::unordered_map<SDL_JoystickID, 
					   std::pair<GameController, GameControllerInputUpdater>> activeControllers_;
};

