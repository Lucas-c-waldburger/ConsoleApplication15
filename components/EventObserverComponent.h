#pragma once
#include <SDL.h>
#include "BaseComponent.h"
#include "../core/Monitoring.h"

class Entity;

struct EventObserver : public BaseComponent<EventObserver, 11>
{
	struct Callback
	{
		using Fn = ReturnSignal(*)(const SDL_Event&, Entity&);

		Fn func = nullptr;
		ReturnSignal status = ReturnSignal::Unknown;
	};

	std::unordered_map<uint32_t, Callback> eventCallbacks;
};
