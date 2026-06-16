#pragma once
#include <concepts>
#include <array>
#include <cassert>
#include <limits>
#include <bit>
#include "../../../ecs/EntityEvents.h"

namespace test {

template <uint64_t num, uint64_t...vals>
struct is_next_pwr_of_two;

template <uint64_t num, uint64_t val>
struct is_next_pwr_of_two<num, val>
{
	static constexpr bool value = num == val;
};

template <uint64_t num, uint64_t val, uint64_t...vals>
struct is_next_pwr_of_two<num, val, vals...>
{
	static constexpr bool value = 
		num == val && is_next_pwr_of_two<num << 1, vals...>::value;
};

template <uint64_t...vals>
static constexpr bool is_next_pwr_of_two_v = is_next_pwr_of_two<1 << 0, vals...>::value;

template <typename E, E...vals> 
	requires (std::is_enum_v<E> && is_next_pwr_of_two_v<vals...>)
class CollisionCategoryTrackerTemplate
{
public:
	constexpr uint16_t operator[](E category) const
	{
		assert((category == vals || ...) && "Category enum value not declared");

		const auto idx = std::countr_zero(category);

		assert(idx >= 0);
		assert(static_cast<size_t>(idx) < categories_.size());

		return categories_[static_cast<size_t>(idx)];
	}

	constexpr void Increment(E category)
	{
		assert((category == vals || ...) && "Category enum value not declared");

		const auto idx = std::countr_zero(category);

		assert(idx >= 0);
		assert(static_cast<size_t>(idx) < categories_.size());

		auto& field = categories_[static_cast<size_t>(idx)];
		if (field < std::numeric_limits<uint16_t>::max())
		{
			++field;
		}

		return true;
	}

	constexpr void Decrement(ObjectCategory::Type category)
	{
		assert((category == vals || ...) && "Category enum value not declared");

		const auto idx = std::countr_zero(category);

		assert(idx >= 0);
		assert(static_cast<size_t>(idx) < categories_.size());

		auto& field = categories_[static_cast<size_t>(idx)];
		if (field > 0)
		{
			--field;
		}	
	}

	constexpr bool Empty() const
	{
		return std::apply([](const auto&...cats) {
			return ((cats == 0) && ...);
		}, categories_);
	}

	constexpr bool operator==(const CollisionCategoryTrackerTemplate&) const = default;

private:
	std::array<uint16_t, sizeof...(vals)> categories_ = {0};
};

} // test 