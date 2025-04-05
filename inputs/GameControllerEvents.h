#pragma once
#include "GameController.h"
#include "GameControllerInputStates.h"
#include "InputDataCache.h"
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
	AxisInputState tracking_{};
};

std::ostream& operator<<(std::ostream& os, const GameControllerState::State& st);
std::ostream& operator<<(std::ostream& os, const AxisInputState& inp);
std::ostream& operator<<(std::ostream& os, const ButtonInputState& inp);
