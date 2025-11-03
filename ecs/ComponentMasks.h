#pragma once
#include "ComponentManager.h"

/* Entity have at least one of these components */
template <typename...Ts>
struct Any {
	static constexpr ComponentSignature GetMask() {
		return (ComponentSignature(0) | ... | Ts::componentBit);
	}
};

/* Entity must not have any of these components */
template <typename...Ts>
struct Exclude {
	static constexpr ComponentSignature GetMask() {
		return (ComponentSignature(0) | ... | Ts::componentBit);
	}
};

namespace detail {
template <typename...> struct is_any_masker : std::false_type {};
template <typename...Ts> struct is_any_masker<Any<Ts...>> : std::true_type {};

template <typename...> struct is_exclude_masker : std::false_type {};
template <typename...Ts> struct is_exclude_masker<Exclude<Ts...>> : std::true_type {};
}

template <typename T> 
inline constexpr bool is_any_masker_v = detail::is_any_masker<T>::value;
template <typename T>
inline constexpr bool is_exclude_masker_v = detail::is_exclude_masker<T>::value;

struct ComponentMasks
{
	ComponentSignature includeMask = 0;
	ComponentSignature excludeMask = 0;
	ComponentSignature anyMask = 0;

	constexpr bool ShouldIncludeEntity(const ComponentSignature sig) const
	{
		if ((sig & includeMask) != includeMask)
		{
			return false;
		}
		if (sig & excludeMask)
		{
			return false;
		}
		if (anyMask != 0 && (sig & anyMask) == 0)
		{
			return false;
		}

		return true;
	}

	template <typename...Ts>
	static constexpr ComponentMasks MakeMasks()
	{
		ComponentMasks masks{};
		(masks.template AddMask<Ts>(), ...);

		return masks;
	}

	template <typename T>
	constexpr void AddMask()
	{
		if constexpr (SomeComponent<T>)
		{
			includeMask |= T::componentBit;
		}
		else if constexpr (is_any_masker_v<T>)
		{
			anyMask |= T::GetMask();
		}
		else if constexpr (is_exclude_masker_v<T>)
		{
			excludeMask |= T::GetMask();
		}
		else
		{
			static_assert(false, "type is not a component or component masker");
		}
	}
};