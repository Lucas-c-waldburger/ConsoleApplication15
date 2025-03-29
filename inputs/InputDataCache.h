#pragma once
#include "../components/GameControllerStateComponent.h"
#include <bitset>

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