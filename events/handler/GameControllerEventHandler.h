#pragma once
#include "../../inputs/controller/GameController.h"
#include "../../inputs/InputDataCache.h"
#include <unordered_map>
#include <set>

class GameControllerEventHandler
{
public:
	GameControllerEventHandler() = default;
	~GameControllerEventHandler();

	void HandleDeviceEvent(const SDL_Event& ev);
	void HandleInputEvent(const SDL_Event& ev);
	void UpdateEntities();

private:
	std::unordered_map<SDL_JoystickID, std::pair<GameController, InputDataCache>> activeControllers_;
};

