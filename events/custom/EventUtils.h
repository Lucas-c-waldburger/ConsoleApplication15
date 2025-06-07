#pragma once
#include "CustomEventData.h"

template <SomeCustomEvent T>
static const T* CastEvent(const SDL_Event& ev)
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

