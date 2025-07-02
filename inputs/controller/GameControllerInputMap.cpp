#include "GameControllerInputMap.h"
#include <cassert>

//constexpr GameControllerInputMap::GameControllerInputMap()
//{
//	for (size_t i = 0; i < fields_.size(); i++)
//	{
//		fields_[i].source = static_cast<GameControllerInputSource>(i);
//	}
//}
//
//constexpr GameControllerInputField& GameControllerInputMap::operator[](GameControllerInputSource source)
//{
//	auto idx = static_cast<std::underlying_type_t<GameControllerInputSource>>(source);
//
//	assert(idx >= 0 && idx <= kGameControllerInputSourceEnd);
//
//	return fields_[static_cast<size_t>(idx)];
//}
//
//constexpr const GameControllerInputField& GameControllerInputMap::operator[](GameControllerInputSource source) const
//{
//	auto idx = static_cast<std::underlying_type_t<GameControllerInputSource>>(source);
//
//	assert(idx >= 0 && idx <= kGameControllerInputSourceEnd);
//
//	return fields_[static_cast<size_t>(idx)];
//}
