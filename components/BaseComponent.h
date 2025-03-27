#pragma once
#include "../TTraits.h"

static constexpr uint64_t kReservedComponentBit = 0;

template <typename Derived, size_t Idx>
struct BaseComponent
{
    static constexpr uint64_t componentBit = 1ull << Idx;
};

template <typename T>
concept ComponentType = requires { T::componentBit; };

template <typename...> struct all_unique_component_idxs : std::true_type {};

template <typename T, typename... Ts>
struct all_unique_component_idxs<T, Ts...>
{
    static constexpr bool value = ((T::componentBit != Ts::componentBit) && ...
        && all_unique_component_idxs<Ts...>::value);
};

template <typename...Ts>
static constexpr bool all_unique_component_idxs_v = all_unique_component_idxs<Ts...>::value;

template <typename...Ts>
static constexpr bool no_idxs_are_reserved_bit_v = ((Ts::componentBit != 1ull << kReservedComponentBit) && ...);

template <typename...Ts>
static constexpr bool no_idxs_greater_than_64_v = (((1ull << 63) <= Ts::componentBit) && ...);

template <ComponentType...Ts> requires (all_unique_component_idxs_v<Ts...> &&
                                        no_idxs_are_reserved_bit_v<Ts...>)
using ComponentTypeList = TypeList<Ts...>;
