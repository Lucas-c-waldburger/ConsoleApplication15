#pragma once
#include <cstdint>

using UserEventTypeId = uint32_t;

inline UserEventTypeId NextUserEventTypeId()
{
	static UserEventTypeId next = 0;
	return next++;
}

template <typename T>
inline UserEventTypeId GetUserEventTypeId()
{
	static UserEventTypeId id = NextUserEventTypeId();
	return id;
}