#pragma once
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <optional>
#include "CommonFunctions.h"

#define STR(x) #x
#define NAME_AND_CALL(fn, ...) #fn, fn(__VA_ARGS__)


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

//struct HashName
//{
//    constexpr HashName() = default;
//    constexpr explicit HashName(std::string_view sv) : value(fnv1aHash(sv)) {}
//    //constexpr explicit HashName(const char* cc) : value(fnv1aHash(cc)) {}
//
//    constexpr HashName& operator=(const HashName& other) 
//    { 
//        if (this != &other)
//        {
//            value = other.value;
//        }
//        return *this;
//    }
//    constexpr HashName& operator=(std::string_view sv)
//    {
//        value = fnv1aHash(sv);
//        return *this;
//    }
//    //constexpr HashName& operator=(const char* cc)
//    //{
//    //    value = fnv1aHash(cc);
//    //    return *this;
//    //}
//
//    constexpr bool operator==(const HashName&) const = default;
//    constexpr bool operator==(std::string_view sv) const
//    {
//        return value == fnv1aHash(sv);
//    }
//
//    uint32_t value = 0;
//}; 
//
//inline constexpr HashName kInvalidHashName{};
//
//namespace std {
//    template <>
//    struct hash<HashName> {
//        size_t operator()(const HashName& hashName) const noexcept {
//            return hashName.value;
//        }
//    };
//}
//
//struct HashNameHash
//{
//    using is_transparent = void;
//
//    size_t operator()(const HashName& hn) const noexcept {
//        return std::hash<HashName>{}(hn);
//    }
//    size_t operator()(const std::string_view sv) const noexcept {
//        return std::hash<HashName>{}(HashName{ sv });
//    }
//};
//
//struct HashNameEq
//{
//    using is_transparent = void;
//
//    bool operator()(const HashName& lhs, const HashName& rhs) const {
//        return lhs == rhs;
//    }
//    bool operator()(const std::string_view sv, const HashName& hn) const {
//        return hn == sv;
//    }
//};
//
//template <typename Value>
//class HashNameMap : public std::unordered_map<HashName, Value, HashNameHash, HashNameEq>
//{
//private:
//    using Super = std::unordered_map<HashName, Value, HashNameHash, HashNameEq>;
//
//public:
//    Value& operator[](std::string_view sv)
//    {
//        return Super::operator[](HashName{ sv });
//    }
//    Value& operator[](const HashName hn)
//    {
//        return Super::operator[](hn);
//    }
//};

//template <typename Key, typename Value, typename...MapArgs> 
//    requires std::is_default_constructible_v<Value>
//class SparseSetTemplate
//{
//public:
//    template <typename T> //requires std::convertible_to<T, Key>
//    Value& operator[](const T& key)
//    {
//        auto it = valueIndicesByKey_.find(key);
//        if (it != valueIndicesByKey_.end())
//        {
//            assert(it->second < values_.size());
//            return values_[it->second];
//        }
//
//        valueIndicesByKey_[key] = values_.size();
//
//        return values_.emplace_back();
//    }
//
//    template <typename T, typename U> //requires (std::convertible_to<T, Key> && 
//                                               // std::convertible_to<U, Value>)
//    bool Emplace(T&& key, U&& val)
//    {
//        if (valueIndicesByKey_.contains(key))
//        {
//            return false;
//        }
//
//        valueIndicesByKey_[std::forward<T>(key)] = values_.size();
//        values_.emplace_back(std::forward<U>(value));
//
//        return true;
//    }
//
//    template <typename T> //requires std::convertible_to<T, Key>
//    bool Erase(const T& key)
//    {
//        auto it = valueIndicesByKey_.find(key);
//        if (it == valueIndicesByKey_.end())
//        {
//            return false;
//        }
//
//        size_t oldIdx = it->second;
//        size_t backIdx = values_.size() - 1;
//        assert(oldIdx <= backIdx);
//
//        auto backKeyIt = valueIndicesByKey_.find()
//    }
//
//    template <typename Fn> requires std::invocable<Fn, Value&>
//    void ForEachValue(Fn&& fn)
//    {
//        auto&& f = std::forward<Fn>(fn);
//        for (auto& val : values_)
//        {
//            std::invoke(f, val);
//        }
//    }
//
//private:
//    std::unordered_map<Key, size_t, MapArgs...> valueIndicesByKey_;
//    std::vector<Value> values_;
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

