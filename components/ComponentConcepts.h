#pragma once
#include "../core/TypeUtils.h"

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
static constexpr bool no_idxs_are_reserved_bit_v = ((Ts::componentBit != 1ull << 0/*kReservedComponentBit*/) && ...);

template <typename...Ts>
static constexpr bool no_idxs_greater_than_64_v = (((1ull << 63) <= Ts::componentBit) && ...);

template <ComponentType...Ts> requires (all_unique_component_idxs_v<Ts...>&&
    no_idxs_are_reserved_bit_v<Ts...>)
    using ComponentTypeList = TypeList<Ts...>;

// ENTITY RETRIEVAL STUFF
template <typename T>
struct Exclude
{
    using WrappedType = std::remove_cvref_t<T>;
};
template <typename T> struct is_exclude : std::false_type {};
template <typename T> struct is_exclude<Exclude<T>> : std::true_type {};

template <typename T>
concept ComponentOrExclusionWrappedType = (ComponentType<T> ||
    (is_exclude<T>::value && ComponentType<typename T::WrappedType>));


//template <ComponentType...Ts>
//struct AnyOf
//{
//    using WrappedTypes = TypeList<Ts...>;
//};
//
//template <typename T> struct is_any_of_wrapper : std::false_type {};
//template <typename...Ts> struct is_any_of_wrapper<AnyOf<Ts...>> : std::true_type {};