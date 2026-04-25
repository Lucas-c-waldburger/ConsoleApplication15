#include "KeyboardEventHandler.h"

void KeyboardEventHandler::HandleKeyboardEvent(float dt, const SDL_Event& ev)
{
	inputUpdater_.Update(dt, ev);
}

void KeyboardEventHandler::Finalize(float dt, EventBus& bus)
{
	inputUpdater_.FinalizeAndPushEvents(dt, bus);
}