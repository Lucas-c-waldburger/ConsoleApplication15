#pragma once
#include "GameController.h"
#include "GameControllerInputStates.h"
#include "InputDataCache.h"
#include <unordered_map>

class GameControllerEventHandler
{
public:
	void HandleDeviceEvent(const SDL_Event& ev);
	void HandleInputEvent(const SDL_Event& ev);
	void UpdateEntities();

private:
	std::unordered_map<SDL_JoystickID, std::pair<GameController, InputDataCache>> activeControllers_;
};

static std::ostream& operator<<(std::ostream& os, const GameControllerState::State& st);
static std::ostream& operator<<(std::ostream& os, const AxisInputState& inp);
static std::ostream& operator<<(std::ostream& os, const ButtonInputState& inp);
