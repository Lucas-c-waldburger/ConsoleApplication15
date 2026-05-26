#pragma once
#include "../../inputs/controller/GameControllerHub.h"
#include "../../inputs/controller/GameControllerInputUpdater.h"
#include "../EventBus2.h"
#include <unordered_map>
#include <set>

class GameControllerEventHandler
{
public:
	//static inline const GameControllerInputMap kInvalidInputMap{};

	using ActiveControllerMap = std::unordered_map<SDL_JoystickID,
		std::pair<GameController, GameControllerInputUpdater>>;

	GameControllerEventHandler() = default;
	~GameControllerEventHandler();

	void HandleDeviceEvent(const SDL_Event& ev, EventBus& bus);
	void HandleInputEvent(const SDL_Event& ev);
	void Finalize(EventBus& bus);

	GameControllerState GetControllerState(SDL_JoystickID joystickId) const;
	//const GameControllerInputMap& GetControllerState(SDL_JoystickID joystickId) const;
	const ActiveControllerMap& GetActiveControllers() const { return activeControllers_; }

	//const GameControllerHub& GetGameControllers() const { return controllerHub_; }
	size_t GetFirstFreeJoystickID() const;

private:
	void UpdateControllerStateComponents();

	ActiveControllerMap activeControllers_;

	//GameControllerHub controllerHub_;
	//std::vector<GameControllerInputUpdater> inputUpdaters_;
};

