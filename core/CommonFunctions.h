#pragma once
#include <iostream>

inline constexpr uint32_t fnv1aHash(std::string_view sv) 
{
    if (sv.empty())
    {
        return 0;
    }

    uint32_t hash = 2166136261u;
    for (char c : sv) 
    {
        hash ^= static_cast<uint8_t>(c);
        hash *= 16777619u;
    }
    return hash;    
}

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