#pragma once
#include <algorithm>
#include <vector>
#include "TypeUtils.h"

namespace core {

template <typename Container, typename Value>
inline bool Erase(Container& c, Value&& val)
{
    const size_t oldSize = c.size();
    c.erase(
        std::remove(std::begin(c), std::end(c), std::forward<Value>(val)),
        std::end(c)
    );

    return c.size() < oldSize;
}

template <typename Container, typename Pred>
inline bool EraseIf(Container& c, Pred&& pred)
{
    const size_t oldSize = c.size();
    c.erase(
        std::remove_if(std::begin(c), std::end(c), std::forward<Pred>(pred)),
        std::end(c)
    );

    return c.size() < oldSize;
}

// TODO: Add concepts to distinguish find() from find_if()
template <typename Container, typename Object>
inline auto Find(Container& c, Object&& obj)
{
    return std::find_if(std::begin(c), std::end(c), std::forward<Object>(obj));
}

template <typename Container, typename Pred>
inline auto FindIf(Container& c, Pred&& pred)
{
    return std::find_if(std::begin(c), std::end(c), std::forward<Pred>(pred));
}

template <typename Container, typename Pred>
inline auto AnyOf(Container& c, Pred&& pred)
{
    return std::any_of(std::begin(c), std::end(c), std::forward<Pred>(pred));
}

template <typename Container, typename Pred>
inline auto AllOf(Container& c, Pred&& pred)
{
    return std::all_of(std::begin(c), std::end(c), std::forward<Pred>(pred));
}

template <typename Container, typename Object>
inline bool Contains(Container& c, Object&& obj)
{
    return Find(c, std::forward<Object>(obj)) != c.end();
}

template <typename Container, typename Pred>
inline bool ContainsIf(Container& c, Pred&& pred)
{
    return FindIf(c, std::forward<Pred>(pred)) != c.end();
}

template <typename T, typename...Us> 
    requires (sizeof...(Us) > 0 && (EqualityComparableTo<T, Us> && ...))
inline constexpr bool EqualsAny(const T& val, const Us&...rest)
{
    return ((val == rest) || ...);
}

} // core