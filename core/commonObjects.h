#pragma once
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <optional>
#include "CommonFunctions.h"
#include "formatters.h"

#define STR(x) #x
#define NAME_AND_CALL(fn, ...) #fn, fn(__VA_ARGS__)


struct Void {};
inline constexpr Void kVoid{};

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
    friend constexpr bool operator==(const Dimensions<T>& lhs,
                                     const Dimensions<T>& rhs)
    {
        return lhs.w == rhs.w && lhs.h == rhs.h;
    }
};

template <typename T>
struct Range
{
    T min;
    T max;
    friend constexpr bool operator==(const Range<T>& lhs,
                                     const Range<T>& rhs)
    {
        return lhs.min == rhs.min && lhs.max == rhs.max;
    }
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
    friend constexpr bool operator==(const HandedPair<T>& lhs, 
                                     const HandedPair<T>& rhs)
    {
        return lhs.left == rhs.left && lhs.right == rhs.right;
    }
};

template <typename T> //requires std::is_default_constructible_v<T>
struct Requestable
{
    T value;
    std::optional<T> requested;
};

template <typename T, typename U = T>
struct ThresholdTracker
{
    T accumulated;
    T threshold;
    DataRecord<U> recorded;
};

template <typename T>
struct Extent
{
    T start;
    T end;
};

// PAIR HASH
namespace std {
template <class T1, class T2>
struct hash<std::pair<T1, T2>> {
    std::size_t operator()(const std::pair<T1, T2>& p) const noexcept {
        std::size_t h1 = std::hash<T1>{}(p.first);
        std::size_t h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
}

// OVERLOADED
template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;


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

// SINGLE VALUE OPTIONAL TUPLE UNWRAPPER

template <typename T>
class MonoValueOptionalTupleUnwrapper
{
public:
    MonoValueOptionalTupleUnwrapper() : tup_(std::nullopt) {}
    explicit MonoValueOptionalTupleUnwrapper(std::optional<std::tuple<T>>&& tup) : tup_(std::move(tup)) {}

    constexpr bool has_value() const noexcept { return tup_.has_value(); }
    constexpr operator bool() const noexcept { return has_value(); }
    constexpr bool operator!() const noexcept { return !has_value(); }

    template <typename U = std::remove_cv_t<T>>
    constexpr T value_or(U&& fallback)
    {
        if (has_value())
        {
            return std::get<0>(*tup_);
        }
        return fallback;
    }
    template <typename U = std::remove_cv_t<T>>
    constexpr T value_or(U&& fallback) const
    {
        if (has_value())
        {
            return std::get<0>(*tup_);
        }
        return fallback;
    }

    constexpr T& operator*() requires !std::is_const_v<std::remove_reference_t<T>> 
    { 
        assert(has_value()); return std::get<0>(*tup_); 
    }
    constexpr const T& operator*() const 
    { 
        assert(has_value()); return std::get<0>(*tup_);
    }


private:
    std::optional<std::tuple<T>> tup_;
};

// NULL
template <typename T> requires std::is_default_constructible_v<T>
inline constexpr const T& Null()
{
    static const T nullValue{};
    return nullValue;
}

