#pragma once
#include <iostream>
#include <cassert>

struct Void {};

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

template <typename T>
struct DirtyFlags
{

};

//template <typename T> requires (std::is_default_constructible_v<T> && 
//                                std::equality_comparable<T>)
//struct DirtyMarker
//{
//    T data{};
//    bool dirty = true;
//
//    constexpr DirtyMarker() = default;
//    constexpr DirtyMarker(const T& val) : data(val) {}
//    constexpr DirtyMarker(T&& val) : data(std::move(val)) {}
//
//    constexpr DirtyMarker& operator=(const T& val)
//    {
//        if (data != val) 
//        { 
//            data = val;
//            dirty = true;
//        }
//        return *this;
//    }
//    constexpr DirtyMarker& operator=(T&& val)
//    {
//        if (data != val)
//        {
//            data = std::move(val);
//            dirty = true;
//        }
//        return *this;
//    }
//
//    constexpr bool operator==(const T& val) const { return data == val; }
//
//    constexpr operator T& () { return data; }
//    constexpr operator const T& () const { return data; }
//};

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

