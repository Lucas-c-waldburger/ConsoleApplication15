#pragma once
#include <cmath>

inline constexpr uint8_t operator "" _u8(unsigned long long value)
{
    return static_cast<uint8_t>(value);
}

inline constexpr uint16_t operator "" _u16(unsigned long long value)
{
    return static_cast<uint16_t>(value);
}

inline constexpr uint32_t operator "" _u32(unsigned long long value)
{
    return static_cast<uint32_t>(value);
}

inline constexpr size_t operator "" _uz(unsigned long long value)
{
    return static_cast<size_t>(value);
}