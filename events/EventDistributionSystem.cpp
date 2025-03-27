#include "EventDistributionSystem.h"
#include "EventBuffer.h"
#include "../inputs/GameControllerEvents.h"
#include <SDL.h>

bool EventDistributionSystem::Distribute(EventBuffer& buffer)
{
	if (buffer.Empty())
	{
		return true;
	}

	while (!buffer.Empty())
	{
		auto ev = buffer.Pop();

		switch (ev.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
			gameControllerHandler_.HandleDeviceEvent(ev);

		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			gameControllerHandler_.HandleInputEvent(ev);

		default:
			break;
		}
	}

	gameControllerHandler_.UpdateEntities();

	return true;
}