#pragma once
#include <array>
#include <cassert>
#include "GameControllerInputField.h"
#include "../../core/SizedEnumMap.h"

using GameControllerInputMap = SizedEnumMap<GameControllerInputSource, GameControllerInputField>;

inline constexpr GameControllerInputMap MakeGameControllerInputMap()
{
	using Source = GameControllerInputSource;

	GameControllerInputMap inputMap{};

	for (size_t i = enum_start_v<Source>; i < enum_size_v<Source>; i++)
	{
		auto src = static_cast<Source>(i);
		inputMap[src].source = src;
	}

	return inputMap;
}

//class GameControllerInputMap 
//{
//public:
//	constexpr GameControllerInputMap()
//	{
//		for (size_t i = 0; i < fields_.size(); i++)
//		{
//			fields_[i].source = static_cast<GameControllerInputSource>(i);
//		}
//	}
//
//	constexpr GameControllerInputField& operator[](GameControllerInputSource source)
//	{
//		auto idx = static_cast<std::underlying_type_t<GameControllerInputSource>>(source);
//
//		assert(idx >= 0 && idx < enum_size_v<GameControllerInputSource>);
//
//		return fields_[static_cast<size_t>(idx)];
//	}
//
//	constexpr const GameControllerInputField& operator[](GameControllerInputSource source) const
//	{
//		auto idx = static_cast<std::underlying_type_t<GameControllerInputSource>>(source);
//
//		assert(idx >= 0 && idx < enum_size_v<GameControllerInputSource>);
//
//		return fields_[static_cast<size_t>(idx)];
//	}
//
//	constexpr size_t Size() const { return fields_.size(); }
//
//private:
//    std::array<GameControllerInputField, enum_size_v<GameControllerInputSource>> fields_;
//};
