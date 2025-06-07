#include "SDLInputSystem.h"
#include "../events/EventBus.h"
#include "../events/custom/data/Groups.h"


bool SDLInputSystem::Update(EventSystem& eventSystem)
{
	SDL_PumpEvents();

	auto systemEvents = EventBus::GetEvents<events::SystemEventGroup>();

	for (const auto& event : systemEvents)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
			gameControllerHandler_.HandleDeviceEvent(event);
			break;

		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			gameControllerHandler_.HandleInputEvent(event);
			break;

		default:
			break;
		}
	}

	gameControllerHandler_.UpdateEntities();

	eventSystem.DispatchEvents<events::SystemEventGroup>();

	return true;
}