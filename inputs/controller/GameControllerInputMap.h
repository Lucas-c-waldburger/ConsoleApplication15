#pragma once
#include <array>
#include <cassert>
#include "GameControllerInputField.h"

class GameControllerInputMap 
{
public:
	constexpr GameControllerInputMap()
	{
		for (size_t i = 0; i < fields_.size(); i++)
		{
			fields_[i].source = static_cast<GameControllerInputSource>(i);
		}
	}

	constexpr GameControllerInputField& operator[](GameControllerInputSource source)
	{
		auto idx = static_cast<std::underlying_type_t<GameControllerInputSource>>(source);

		assert(idx >= 0 && idx <= kGameControllerInputSourceEnd);

		return fields_[static_cast<size_t>(idx)];
	}

	constexpr const GameControllerInputField& operator[](GameControllerInputSource source) const
	{
		auto idx = static_cast<std::underlying_type_t<GameControllerInputSource>>(source);

		assert(idx >= 0 && idx <= kGameControllerInputSourceEnd);

		return fields_[static_cast<size_t>(idx)];
	}

	constexpr size_t Size() const { return fields_.size(); }

private:
    std::array<GameControllerInputField, kGameControllerInputSourceEnd + 1> fields_;
};
