#include "SDLInputSystem.h"
#include "../events/EventBus.h"


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
			gameControllerHandler_.HandleDeviceEvent(ev);
			break;

		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			gameControllerHandler_.HandleInputEvent(ev);
			break;

		default:
			break;
		}
	}

	gameControllerHandler_.UpdateEntities();


	return true;
}