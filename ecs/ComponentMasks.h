#pragma once
#include "ComponentManager.h"
#include "../user/UserComponentBridge.h"

/* Entity have at least one of these components */
template <typename...Ts>
struct Any {
	static constexpr ComponentSignature GetMask() {
		return (ComponentSignature(0) | ... | Ts::componentBit);
	}
	static ComponentSignature GetMask(const UserComponentBridge& bridge) 
	{
		auto getBit = [&bridge]<typename T> {
			if constexpr (SomeComponent<T>)
			{
				return T::componentBit;
			}
			else
			{
				return bridge.GetComponentDataSignature<T>();
			}
		};

		return (ComponentSignature(0) | ... | getBit.template operator()<Ts>());
	}
};

/* Entity must not have any of these components */
template <typename...Ts>
struct Exclude {
	static constexpr ComponentSignature GetMask() {
		return (ComponentSignature(0) | ... | Ts::componentBit);
	}
	static ComponentSignature GetMask(const UserComponentBridge& bridge)
	{
		auto getBit = [&bridge]<typename T> {
			if constexpr (SomeComponent<T>)
			{
				return T::componentBit;
			}
			else
			{
				return bridge.GetComponentDataSignature<T>();
			}
		};

		return (ComponentSignature(0) | ... | getBit.template operator()<Ts>());
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
	static ComponentMasks MakeMasks()
	{
		ComponentMasks masks{};
		(masks.template AddMask<Ts>(), ...);

		return masks;
	}

	template <typename...Ts>
	static ComponentMasks MakeMasks(const UserComponentBridge& bridge)
	{
		ComponentMasks masks{};
		(masks.template AddMask<Ts>(bridge), ...);

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

	template <typename T>
	void AddMask(const UserComponentBridge& bridge)
	{
		if constexpr (SomeComponent<T>)
		{
			includeMask |= T::componentBit;
		}
		else if constexpr (is_any_masker_v<T>)
		{
			anyMask |= T::GetMask(bridge);
		}
		else if constexpr (is_exclude_masker_v<T>)
		{
			excludeMask |= T::GetMask(bridge);
		}
		else
		{
			includeMask |= bridge.GetComponentDataSignature<T>();
		}
	}
};