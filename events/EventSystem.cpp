#include "EventSystem.h"
#include <SDL.h>

bool EventSystem::Poll(SDL_Event& ev)
{
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT)
		{
			return false;
		}
		eventBuffer_.Push(ev);
	}

	return true;
}

void EventSystem::DistributeEvents()
{
	if (eventBuffer_.Empty())
	{
		return;
	}

	while (!eventBuffer_.Empty())
	{
		auto ev = eventBuffer_.Pop();

		switch (ev.type)
		{
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
}