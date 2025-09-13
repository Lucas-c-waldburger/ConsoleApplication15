#pragma once
#include "SizedEnum.h"
#include <array>
#include <cassert>

template <SomeSizedEnum EnumKey, typename Value>
class SizedEnumMap
{
public:
	constexpr SizedEnumMap() = default;

	constexpr Value& operator[](EnumKey key)
	{
		auto idx = static_cast<std::underlying_type_t<EnumKey>>(key);

		assert(idx >= 0 && idx < enum_size_v<EnumKey>);

		return map_[static_cast<size_t>(idx)];
	}

	constexpr const Value& operator[](EnumKey key) const
	{
		auto idx = static_cast<std::underlying_type_t<EnumKey>>(key);

		assert(idx >= 0 && idx < enum_size_v<EnumKey>);

		return map_[static_cast<size_t>(idx)];
	}

	constexpr Value& operator[](size_t idx)
	{
		return map_[idx];
	}

	constexpr const Value& operator[](size_t idx) const
	{
		return map_[idx];
	}

private:
	std::array<Value, enum_size_v<EnumKey>> map_;
};