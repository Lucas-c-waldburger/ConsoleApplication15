#pragma once
#include <numeric>
#include "GameControllerInputSource.h"
#include "../InputField.h"

struct GameControllerInputFieldValue
{
	int trigger = 0;
	SDL_Point axis = { 0, 0 };
};

using GameControllerInputField = InputField<GameControllerInputSource, 
										    GameControllerInputFieldValue>;

//struct GameControllerInputField
//{
//	GameControllerInputSource source = GameControllerInputSource::Invalid;
//	InputState state = InputState::None;
//	uint32_t stateDuration = 0;
//	GameControllerInputFieldValue value;
//};