#pragma once
#include <cstdint>

using UserEventTypeId = uint32_t;

inline UserEventTypeId NextUserEventTypeId()
{
	static UserEventTypeId next = 0;
	return next++;
}

template <typename T> requires std::same_as<T, std::remove_cvref_t<T>>
inline UserEventTypeId GetUserEventTypeId()
{
	static UserEventTypeId id = NextUserEventTypeId();
	return id;
}