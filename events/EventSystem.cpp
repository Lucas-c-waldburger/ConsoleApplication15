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

		//if (ev.type == SDL_CONTROLLERAXISMOTION)
		//{
		//	LOG_ERROR("Controller Axis Motion Event from SDL_PollEvents!");
		//}

		eventBuffer_.Push(ev);
	}

	return true;
}

void EventSystem::DistributeEvents()
{
	//if (eventBuffer_.Empty())
	//{
	//	return;
	//}

	while (!eventBuffer_.Empty())
	{
		auto ev = eventBuffer_.Pop();
		assert(ev.type != SDL_POLLSENTINEL);
		//LOG_WARNING_FMT("Event Buffer Size: {}", eventBuffer_.Size());

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
			//LOG_WARNING("Axis Event Handled!");
			break;

		default:
			break;
		}
	}
	//LOG_WARNING("Done Distributing Events");
	gameControllerHandler_.UpdateEntities();
}