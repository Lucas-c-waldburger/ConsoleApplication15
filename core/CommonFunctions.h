#pragma once
#include <iostream>

inline void HashCombine(size_t& seed, size_t value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <std::floating_point T>
constexpr bool EqualsWithTolerance(T a, T b, T absEpsilon = static_cast<T>(1e-5),
                                             T relEpsilon = static_cast<T>(1e-4))
{
    T diff = std::abs(a - b);
    if (diff <= absEpsilon)
    {
        return true;
    }

    return diff <= std::max(std::abs(a), std::abs(b)) * relEpsilon;
}