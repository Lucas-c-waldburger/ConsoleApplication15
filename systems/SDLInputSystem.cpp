#include "SDLInputSystem.h"
#include "../FeatureFlags.h"
#include "../events/data/GameControllerEvents.h"
#include "../atlas/NewTextureRepository.h"

#if IMGUI_ENABLED
#include "../gui/GuiContext.h"
#endif

bool SDLInputSystem::Update(float dt, EventBus& bus, 
							TextureRepository& repo, SDL_Renderer* renderer)
{
	while (SDL_PollEvent(&sdlEvent_))
	{
#if IMGUI_ENABLED
		if (GuiContext::IsInitialized())
		{
			GuiContext::ProcessEvent(sdlEvent_);
		}
#endif
		switch (sdlEvent_.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_RENDER_TARGETS_RESET:
		case SDL_RENDER_DEVICE_RESET:
			bus.PushEvent(events::RenderReset{});
			break;

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

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			keyboardHandler_.HandleKeyboardEvent(dt, sdlEvent_);
			break;

		default:
			break;
		}
	}

	gameControllerHandler_.Finalize(bus);
	mouseHandler_.Finalize(dt, bus);
	keyboardHandler_.Finalize(dt, bus);

	bus.DispatchEvents();

	SDL_zero(sdlEvent_);

	return true;
}