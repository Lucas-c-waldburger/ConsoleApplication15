#pragma once
#include <iostream>
#include <cassert>
#include "CommonFunctions.h"

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
};

template <typename T>
struct HandedPair
{
    T left;
    T right;
};

struct HashName
{
    constexpr HashName() = default;
    constexpr explicit HashName(std::string_view sv) : value(fnv1aHash(sv)) {}

    constexpr bool operator==(const HashName& other) const = default;
    constexpr bool operator==(std::string_view raw) const
    {
        return value == fnv1aHash(raw);
    }

    uint32_t value = 0;
};

namespace std {
    template <>
    struct hash<HashName> {
        size_t operator()(const HashName& hashName) const noexcept {
            return hashName.value;
        }
    };
}

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

