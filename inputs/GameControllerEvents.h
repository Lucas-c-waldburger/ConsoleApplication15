#pragma once
#include "GameController.h"
#include "../Components.h"
#include <bitset>


//class EventDomainManager
//{
//public:
//	virtual void HandleEvent(const SDL_Event&) = 0;
//	virtual void UpdateEntities() = 0;
//  virtual bool InDomain(const SDL_Event&) = 0;
//
//private:
//};

class GameControllerEventHandler
{
public:
	struct InputDataCache
	{
		// extends the SDL_Button-based bitfield to include axis updates
		static constexpr uint8_t LeftAxisIndex = SDL_CONTROLLER_BUTTON_MAX;
		static constexpr uint8_t RightAxisIndex = LeftAxisIndex + 1;
		static constexpr uint8_t InputMax = RightAxisIndex + 1;

		void UpdateSkippedInputs();
		static uint8_t GetAxisIndexForEnum(uint8_t axisEnum);

		std::bitset<InputMax> inputUpdatedTracker;
		GameControllerState cachedControllerState;
	};

	void HandleDeviceEvent(const SDL_Event& ev);
	void HandleInputEvent(const SDL_Event& ev);
	void UpdateEntities();

private:
	std::unordered_map<SDL_JoystickID, std::pair<GameController, InputDataCache>> activeControllers_;
};

static std::ostream& operator<<(std::ostream& os, const GameControllerState::State& st);
static std::ostream& operator<<(std::ostream& os, const AxisInputState& inp);
static std::ostream& operator<<(std::ostream& os, const ButtonInputState& inp);
