#pragma once
#include <unordered_map>
#include <vector>
#include "GameController.h"
#include "../../components/GameControllerStateComponent.h"

class GameControllerHub
{
public:
	friend class GameControllerEventHandler;

	GameControllerHub() = default;
	~GameControllerHub();
	GameControllerHub(const GameControllerHub&) = delete;
	GameControllerHub& operator=(const GameControllerHub&) = delete;
	GameControllerHub(GameControllerHub&& other) noexcept;
	GameControllerHub& operator=(GameControllerHub&& other) noexcept;

	Result<SDL_JoystickID> Connect(Sint32 deviceIdx);
	bool Disconnect(SDL_JoystickID joystickId);
	bool IsConnected(SDL_JoystickID joystickId) const;
	const GameControllerState& GetControllerState(SDL_JoystickID joystickId) const;

private:
	static inline GameControllerState kInvalidGameControllerState{};

	std::unordered_map<SDL_JoystickID, size_t> joystickIdToControllerIndex_;
	std::vector<GameController> gameControllers_;
	std::vector<GameControllerState> controllerStates_;
};
