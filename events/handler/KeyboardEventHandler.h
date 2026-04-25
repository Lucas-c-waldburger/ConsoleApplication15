#pragma once
#include "../../inputs/keyboard/KeyboardInputUpdater.h"

class EventBus;

class KeyboardEventHandler
{
public:
	KeyboardEventHandler() = default;
	~KeyboardEventHandler() = default;

	void HandleKeyboardEvent(float dt, const SDL_Event& ev);
	void Finalize(float dt, EventBus& bus);

private:
	KeyboardInputUpdater inputUpdater_;
};
