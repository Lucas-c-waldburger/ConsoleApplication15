#pragma once
#include <cstdint>

using UserComponentTypeId = uint32_t;

inline UserComponentTypeId NextUserComponentTypeId()
{
	static UserComponentTypeId next = 0;
	return next++;
}

template <typename T>
inline UserComponentTypeId GetUserComponentTypeId()
{
	static UserComponentTypeId id = NextUserComponentTypeId();
	return id;
}