#pragma once
#include <SDL.h>
#include <typeindex>
#include "BaseComponent.h"
#include "../core/Monitoring.h"
#include "../events/EventCallbackMap.h"

class Entity;

struct EventObserver : public BaseComponent<EventObserver, 11>
{
	struct Callback
	{
		//using Fn = ReturnSignal(*)(const SDL_Event&, Entity&);
		using Fn = std::function<ReturnSignal(const SDL_Event&, Entity&)>;

		Fn func = nullptr;
		ReturnSignal status = ReturnSignal::Unknown;
	};

	std::unordered_map<uint32_t, Callback> eventCallbacks;
};

struct EventCallbacks : BaseComponent<EventCallbacks, 14>
{
	EventCallbackMap map;  
};
