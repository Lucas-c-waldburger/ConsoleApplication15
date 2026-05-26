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

inline constexpr void HashCombine(size_t& seed, size_t value) noexcept
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template <typename...Ts> requires (Hashable<Ts> && ...)
inline constexpr size_t MakeHash(const Ts&...args)
{
    size_t hash = 0;
    ((HashCombine(hash, std::hash<Ts>(args))), ...);

    return hash;
}


template <typename...Ts>
inline constexpr size_t TypeIdHash() 
{
    size_t hash = 0;
    ((HashCombine(hash, typeid(Ts).hash_code())), ...);

    return hash;
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