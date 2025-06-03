#pragma once
#include "../../core/commonObjects.h"
#include "CustomEventData.h"

class CustomEvents
{ 
public:
	template <SomeCustomEvent T>
	static SDL_Event MakeNewEvent(T&& evData)
	{
		SDL_Event ev{};
		ev.type = T::GetEventType();
		ev.user.data1 = nullptr;
		ev.user.data2 = nullptr;
		ev.user.code = static_cast<Sint32>(T::eventCode);
	 
		if (ev.type == kInvalidEventType)
		{
			LOG_WARNING("Invalid event type for template arg, did you register it?");
			return ev;
		}

		ev.user.data1 = new T{ std::forward<T>(evData) };

		return ev; 
	}

	template <SomeCustomEvent T>
	static Result<Void> FreeEvent(SDL_Event& ev)
	{
		if (ev.type != T::GetEventType())
		{
			return MAKE_ERROR("Event type inside SDL_Event did not match template arg event type");
		}
		if (!ev.user.data1)
		{
			LOG_WARNING("user.data1 was null");
			return Void{};
		}

		delete static_cast<T*>(ev.user.data1);
		ev.user.data1 = nullptr;

		return Void{};
	}

	template <SomeCustomEvent T>
	static const T* GetEventData(const SDL_Event& ev)
	{
		if (ev.type != T::GetEventType())
		{
			LOG_WARNING("Event type inside SDL_Event did not match template arg event type");
			return nullptr;
		}
		if (!ev.user.data1)
		{
			LOG_WARNING("user.data1 was null");
			return nullptr;
		}

		return static_cast<const T*>(ev.user.data1);
	}
};


