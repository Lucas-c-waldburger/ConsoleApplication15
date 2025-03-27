#pragma once
#include <iostream>
#include <cassert>
#include "components/BaseComponent.h"

using Entity_t = uint32_t;

static constexpr Entity_t kMaxEntities = 500;
static constexpr Entity_t kInvalidEntity = -1;
static constexpr size_t kMaxComponents = 64;
static constexpr size_t kInvalidIndex = kMaxEntities + 1;

using ComponentSignature = uint64_t;

//struct ComponentBitFactory
//{
//    template <typename T> friend struct BaseComponent;
//private:
//    static uint64_t GetNextComponentBit()
//    {
//        static uint64_t bitCounter = 1;
//        uint64_t bit = bitCounter;
//        bitCounter <<= 1;
//
//        return bit;
//    }
//};
//
//template <typename Derived>
//struct BaseComponent
//{
//    static inline const uint64_t componentBit = ComponentBitFactory::GetNextComponentBit();
//};
//
//template <typename T>
//concept ComponentType = std::same_as<std::remove_cvref_t<decltype(T::componentBit)>, uint64_t>;

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

template <typename Base, ComponentType...Ts> requires (std::derived_from<Ts, Base> && ...)
struct UpcastTo
{
    static_assert(sizeof...(Ts) > 0);
    using WrappedDerivedTypes = TypeList<Ts...>;
    using WrappedBaseType = Base;
};

template <typename T> struct is_upcast_to_wrapper : std::false_type {};
template <typename T, typename...Ts> struct is_upcast_to_wrapper<UpcastTo<T, Ts...>> : std::true_type {};

template <ComponentType...Ts>
struct AnyOf
{
    using WrappedTypes = TypeList<Ts...>;
};

template <typename T> struct is_any_of_wrapper : std::false_type {};
template <typename...Ts> struct is_any_of_wrapper<AnyOf<Ts...>> : std::true_type {};


template <typename Tup, typename Fn, size_t... Is>
static void ForEachInTupleImpl(Tup&& tup, Fn&& fn, std::index_sequence<Is...>)
{
    (fn(std::get<Is>(std::forward<Tup>(tup))), ...);
}

template <typename Tup, typename Fn>
static void ForEachInTuple(Tup&& tup, Fn&& fn)
{
    ForEachInTupleImpl(std::forward<Tup>(tup), std::forward<Fn>(fn),
        std::make_index_sequence<std::tuple_size<std::remove_cvref_t<Tup>>::value>{});
}

template <typename T> requires std::is_arithmetic_v<T>
struct Dimensions
{
    T w = static_cast<T>(0);
    T h = static_cast<T>(0);

    constexpr bool operator==(const Dimensions&) const = default;
};

template <typename T>
struct Range
{
    T min;
    T max;
};

template <typename T>
struct DataRecord
{
    T last;
    T now;

    void Update(T newVal)
    {
        last = std::move(now);
        now = std::move(newVal);
    }
};

template <typename T>
struct HandedPair
{
    T left;
    T right;
};

static constexpr int GetNextPowerOfTwo(int x)
{
    if (x <= 0) return 1;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return x + 1;
}


