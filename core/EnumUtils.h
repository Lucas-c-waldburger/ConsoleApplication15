#pragma once
#include <concepts>

// specialize to true for scoped enums that you want to enable bitwise operators for
template <typename E> requires std::is_scoped_enum_v<E>
inline constexpr bool enable_bitwise_operators_v = false;

template <typename E> requires enable_bitwise_operators_v<E>
constexpr E operator|(E lhs, E rhs) noexcept
{
    using U = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

template <typename E> requires enable_bitwise_operators_v<E>
constexpr E operator&(E lhs, E rhs) noexcept
{
    using U = std::underlying_type_t<E>;
    return static_cast<E>(
        static_cast<U>(lhs) & static_cast<U>(rhs));
}

template <typename E> requires enable_bitwise_operators_v<E>
constexpr E operator^(E lhs, E rhs) noexcept
{
    using U = std::underlying_type_t<E>;
    return static_cast<E>(
        static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

template <typename E> requires enable_bitwise_operators_v<E>
constexpr E operator~(E value) noexcept
{
    using U = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<U>(value));
}

template <typename E> requires enable_bitwise_operators_v<E>
constexpr E& operator|=(E& lhs, E rhs) noexcept
{
    return lhs = lhs | rhs;
}

template <typename E> requires enable_bitwise_operators_v<E>
constexpr E& operator&=(E& lhs, E rhs) noexcept
{
    return lhs = lhs & rhs;
}

template <typename E> requires enable_bitwise_operators_v<E>
constexpr E& operator^=(E& lhs, E rhs) noexcept
{
    return lhs = lhs ^ rhs;
}