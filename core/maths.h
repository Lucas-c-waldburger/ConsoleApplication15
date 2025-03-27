#pragma once
#include <cmath>
#include <algorithm>

template <typename T> requires (std::is_arithmetic_v<T>)
static constexpr T Abs(const T x) { return (x < 0) ? -x : x; }

template <typename T> requires (std::is_arithmetic_v<T>)
static constexpr T Max(const T a, const T b) { return (a > b) ? a : b; }

template <typename T, typename U> requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
static constexpr float GetPercentDifference(const T a, const U b)
{
	constexpr float af = static_cast<float>(a);
	constexpr float bf = static_cast<float>(b);

	return (Abs(af - bf) / Max(af, bf)) * 100.0;
}