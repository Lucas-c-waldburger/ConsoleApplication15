#pragma once
#include "Event.h"
#include "EventConcepts.h"

template <SomeEventData T>
inline const T* EventDataCast(const Event& event)
{
    return (event.type == T::eventType) ? static_cast<const T*>(event.data) : nullptr;
}


