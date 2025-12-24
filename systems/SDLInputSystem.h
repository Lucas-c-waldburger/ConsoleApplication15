#pragma once
#include "System.h"
#include "../events/handler/GameControllerEventHandler.h"
#include "../events/handler/MouseEventHandler.h"
#include "../events/EventBus2.h"

class TextureRepository;

class SDLInputSystem : public System
{
public:
	SDLInputSystem() { SDL_zero(sdlEvent_); }

	bool Update(float delta, EventBus2& bus, 
				TextureRepository& repo, SDL_Renderer* renderer);

	const GameControllerEventHandler& 
	GetGameControllerEventHandler() const { return gameControllerHandler_; }

	const MouseEventHandler& GetMouseEventHandler() const { return mouseHandler_; }

private:
	SDL_Event sdlEvent_;
	GameControllerEventHandler gameControllerHandler_;
	MouseEventHandler mouseHandler_;
};

