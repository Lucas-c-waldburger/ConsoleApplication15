#pragma once
#include <algorithm>
#include <vector>

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

template <typename Container, typename Pred>
inline auto FindIf(Container& c, Pred&& pred)
{
    return std::find_if(std::begin(c), std::end(c), std::forward<Pred>(pred));
}