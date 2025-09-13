#include "SDLInputSystem.h"
#include "../events/EventBus.h"
#include "../events/data/GameControllerEvents.h"

bool SDLInputSystem::Update(float delta, EventBus2& bus)
{
	while (SDL_PollEvent(&sdlEvent_))
	{
		switch (sdlEvent_.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
			gameControllerHandler_.HandleDeviceEvent(sdlEvent_, bus);
			break;

		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			gameControllerHandler_.HandleInputEvent(sdlEvent_);
			break;

		case SDL_MOUSEMOTION:
		case SDL_MOUSEWHEEL:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			mouseHandler_.HandleMouseEvent(sdlEvent_);
			break;

		default:
			break;
		}
	}

	gameControllerHandler_.Finalize(bus);
	mouseHandler_.Finalize(delta, bus);

	bus.DispatchEvents();

	return true;
}