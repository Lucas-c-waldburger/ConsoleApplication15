#pragma once
#include "../../inputs/controller/GameController.h"
#include "../../inputs/controller/GameControllerInputUpdater.h"
#include <unordered_map>
#include <set>

class GameControllerEventHandler
{
public:
	GameControllerEventHandler() = default;
	~GameControllerEventHandler();

	void HandleDeviceEvent(const SDL_Event& ev);
	void HandleInputEvent(const SDL_Event& ev);
	void Finalize();

private:
	void UpdateControllerStateComponents();

	std::unordered_map<SDL_JoystickID, 
					   std::pair<GameController, GameControllerInputUpdater>> activeControllers_;
};

