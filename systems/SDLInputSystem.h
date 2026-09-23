#pragma once
#include "System.h"
#include "../events/handler/GameControllerEventHandler.h"
#include "../events/handler/MouseEventHandler.h"
#include "../events/handler/KeyboardEventHandler.h"
#include "../events/EventBus2.h"
#include "../render/RenderTarget.h"

class SDLInputSystem : public System
{
public:
	struct ResourceContext
	{
		EventBus& bus;
		Dimensions<int> renderTargetDimensions = { 0, 0 };
		SDL_FRect displayArea = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	SDLInputSystem() { SDL_zero(sdlEvent_); }

	bool Update(float dt, EventBus& bus, const RenderTargetState& renderTargetState);

	const GameControllerEventHandler& 
	GetGameControllerEventHandler() const { return gameControllerHandler_; }
	const MouseEventHandler& GetMouseEventHandler() const { return mouseHandler_; }
	const KeyboardEventHandler& GetKeyboardEventHandler() const { return keyboardHandler_; }

private:
	SDL_Event sdlEvent_;
	GameControllerEventHandler gameControllerHandler_;
	MouseEventHandler mouseHandler_;
	KeyboardEventHandler keyboardHandler_;
};

