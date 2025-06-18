#include "SDLInputSystem.h"
#include "../events/EventBus.h"
#include "../events/data/GameControllerEvents.h"

bool SDLInputSystem::Update()
{
	while (SDL_PollEvent(&sdlEvent_))
	{
		switch (sdlEvent_.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
			gameControllerHandler_.HandleDeviceEvent(sdlEvent_);
			break;

		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			gameControllerHandler_.HandleInputEvent(sdlEvent_);
			break;

		default:
			break;
		}
	}

	EventBus::DispatchEventGroup<events::GameControllerEventGroup>();

	gameControllerHandler_.UpdateEntities();

	return true;
}