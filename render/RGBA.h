#pragma once
#include "../core/TypeUtils.h"
#include <algorithm>

template <typename T, typename U> requires (sizeof(T) <= sizeof(U))
inline constexpr T ClampToLimits(U val)
{
    return static_cast<T>(
        std::clamp(val, static_cast<U>(std::numeric_limits<T>::min()),
                        static_cast<U>(std::numeric_limits<T>::max())
        ));
}

// an integer size version of SDL_Color so we can perform 
// intermediate math operations on it without being restrained to 0 - 255
struct RGB
{
	int r = 255;
	int g = 255;
	int b = 255;

	template <typename T> requires std::is_arithmetic_v<T>
	using RgbScalarOp = T(*)(int, T);

	template <typename T> requires std::is_arithmetic_v<T>
	static constexpr RGB PerformRgbScalarOp(const RGB& lhs, const T& scalar,
											RgbScalarOp<T> op)
	{
		if constexpr (sizeof(T) < sizeof(int))
		{
			return {
				static_cast<int>(op(ClampToLimits<T>(lhs.r), scalar)),
				static_cast<int>(op(ClampToLimits<T>(lhs.g), scalar)),
				static_cast<int>(op(ClampToLimits<T>(lhs.b), scalar))
			};
		}
		else
		{
			return {
				ClampToLimits<int>(op(static_cast<T>(lhs.r), scalar)),
				ClampToLimits<int>(op(static_cast<T>(lhs.g), scalar)),
				ClampToLimits<int>(op(static_cast<T>(lhs.b), scalar))
			};
		}
	}

	friend constexpr bool operator==(const RGB& lhs, const RGB& rhs)
	{
		return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
	}
	friend constexpr RGB operator+(const RGB& lhs, const RGB& rhs)
	{
		return { lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b };
	}
	friend constexpr RGB operator-(const RGB& lhs, const RGB& rhs)
	{
		return { lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b };
	}
	template <typename T> requires std::is_arithmetic_v<T>
	friend constexpr RGB operator*(const RGB& lhs, const T& scalar)
	{
		return PerformRgbScalarOp(lhs, scalar, +[](int i, T t) { return i * t; });
	}
};


//// an integer size version of SDL_Color so we can perform 
//// intermediate math operations on it without being restrained to 0 - 255
//struct RGBA
//{
//	int r = 255;
//	int g = 255;
//	int b = 255;
//	int a = 255;
//
//	template <typename T> requires std::is_arithmetic_v<T>
//	using RgbaScalarOp = T(*)(int, T);
//
//	template <typename T> requires std::is_arithmetic_v<T>
//	static constexpr RGBA PerformRgbaScalarOp(const RGBA& lhs, const T& scalar, 
//											  RgbaScalarOp<T> op)
//	{
//		if constexpr (sizeof(T) < int)
//		{
//			return {
//				static_cast<int>(op(ClampToLimits<T>(lhs.r), scalar)),
//				static_cast<int>(op(ClampToLimits<T>(lhs.g), scalar)),
//				static_cast<int>(op(ClampToLimits<T>(lhs.b), scalar)),
//				static_cast<int>(op(ClampToLimits<T>(lhs.a), scalar))
//			};
//		}
//		else
//		{
//			return {
//				ClampToLimits<int>(op(static_cast<T>(lhs.r), scalar)),
//				ClampToLimits<int>(op(static_cast<T>(lhs.g), scalar)),
//				ClampToLimits<int>(op(static_cast<T>(lhs.b), scalar)),
//				ClampToLimits<int>(op(static_cast<T>(lhs.a), scalar))
//			};
//		}
//	}
//
//	static constexpr SDL_Color ToSDLColor(const RGBA& rgba)
//	{
//		return SDL_Color{
//			.r = ClampToLimits<uint8_t>(rgba.r),
//			.g = ClampToLimits<uint8_t>(rgba.g),
//			.b = ClampToLimits<uint8_t>(rgba.b),
//			.a = ClampToLimits<uint8_t>(rgba.a)
//		};
//	}
//
//	friend constexpr bool operator==(const RGBA& lhs, const RGBA& rhs)
//	{
//		return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
//	}
//	friend constexpr RGBA operator+(const RGBA& lhs, const RGBA& rhs)
//	{
//		return { lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a };
//	}
//	friend constexpr RGBA operator-(const RGBA& lhs, const RGBA& rhs)
//	{
//		return { lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a };
//	}
//	template <typename T> requires std::is_arithmetic_v<T>
//	friend constexpr RGBA operator*(const RGBA& lhs, const T& scalar)
//	{
//		return PerformRgbaScalarOp(lhs, scalar, &operator*);
//
//		/*if constexpr (sizeof(T) < int)
//		{
//			return {
//				static_cast<int>(ClampToLimits<T>(lhs.r) * scalar),
//				static_cast<int>(ClampToLimits<T>(lhs.g) * scalar),
//				static_cast<int>(ClampToLimits<T>(lhs.b) * scalar),
//				static_cast<int>(ClampToLimits<T>(lhs.a) * scalar)
//			};
//		}
//		else
//		{
//			return {
//				ClampToLimits<int>(static_cast<T>(lhs.r) * scalar),
//				ClampToLimits<int>(static_cast<T>(lhs.g) * scalar),
//				ClampToLimits<int>(static_cast<T>(lhs.b) * scalar),
//				ClampToLimits<int>(static_cast<T>(lhs.a) * scalar)
//			};
//		}*/
//	}
//};


//namespace detail {
//template <typename T, typename U> 
//    requires (std::is_integral_v<T> && std::is_integral_v<U>)
//struct safe_common_integral
//{
//    using type = std::conditional_t<
//        (std::is_unsigned_v<T> && std::is_unsigned_v<U>),
//        size_t,
//        long long
//    >;
//};
//} // detail
//
//template <typename T, typename U>
//using safe_common_integral_type_t = typename detail::safe_common_integral<T, U>::type;
//
//template <typename T, typename U>
//    requires (std::is_integral_v<T> && std::is_integral_v<U>)
//inline constexpr auto WideCommonCast(T a, U b)
//{
//    using Common = safe_common_integral_type_t<T, U>;
//
//    if constexpr (std::same_as<Common, size_t>)
//    {
//        return std::make_pair(static_cast<size_t>(a), static_cast<size_t>(b));
//    }
//    else // Common == long long, have to clamp the one that's unsigned if too big
//    {
//        long long aLong;
//        long long bLong;
//
//        if constexpr (std::is_unsigned_v<T>)
//        {
//            aLong = (a > static_cast<size_t>(std::numeric_limits<long long>::max()))
//                ? std::numeric_limits<long long>::max()
//                : static_cast<long long>(a);
//            bLong = static_cast<long long>(b);
//        }
//        else
//        {
//            aLong = static_cast<long long>(a);
//            bLong = (b > static_cast<size_t>(std::numeric_limits<long long>::max()))
//                ? std::numeric_limits<long long>::max()
//                : static_cast<long long>(b);
//        }
//
//        return std::make_pair(aLong, bLong);
//    }
//}
//
//template <typename T, typename U> requires (sizeof(T) <= sizeof(U))
//inline constexpr T ClampToLimits(U val)
//{
//    return static_cast<T>(
//        std::clamp(val, static_cast<U>(std::numeric_limits<T>::min()),
//                        static_cast<U>(std::numeric_limits<T>::max())
//        ));
//}
//
//template <typename T> requires (std::is_integral_v<T>)
//inline constexpr T SafeIntegralAdd(T a, T b) 
//{
//    if constexpr (std::is_signed_v<T>) 
//    {
//        if ((b > static_cast<T>(0)) && (a > std::numeric_limits<T>::max() - b))
//        {
//            return std::numeric_limits<T>::max();
//        }
//        if ((b < static_cast<T>(0)) && (a < std::numeric_limits<T>::min() - b))
//        {
//            return std::numeric_limits<T>::min();
//        }
//    }
//    else 
//    {
//        if (a > std::numeric_limits<T>::max() - b)
//        {
//            return std::numeric_limits<T>::max();
//        }
//    }
//
//    return a + b;
//}
//
//template <typename T> requires (std::is_integral_v<T>)
//inline constexpr T SafeIntegralSubtract(T a, T b) 
//{
//    if constexpr (std::is_signed_v<T>) 
//    {
//        if ((b < static_cast<T>(0)) && (a > std::numeric_limits<T>::max() + b))
//        {
//            return std::numeric_limits<T>::max();
//        }
//        if ((b > static_cast<T>(0)) && (a < std::numeric_limits<T>::min() + b))
//        {
//            return std::numeric_limits<T>::min();
//        }
//    }
//    else 
//    {
//        if (a < b)
//        {
//            return static_cast<T>(0);
//        }
//    }
//
//    return a - b;
//}
//
//template <typename T> requires std::is_arithmetic_v<T>
//struct RGBA
//{
//    T r = static_cast<T>(255);
//    T g = static_cast<T>(255);
//    T b = static_cast<T>(255);
//    T a = static_cast<T>(255);
//
//    friend constexpr RGBA operator+(const RGBA& lhs, const RGBA& rhs)
//    {
//        return { lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a };
//    }
//    friend constexpr RGBA operator-(const RGBA& lhs, const RGBA& rhs)
//    {
//        return { 
//            std::clamp(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a };
//
//    }
//
//    template <typename U> requires (std::convertible_to<T, U>&&
//        std::is_arithmetic_v<U>)
//        friend constexpr RGB operator*(const RGB& lhs, const U& rhs)
//    {
//        U newR = static_cast<U>(lhs.r) * rhs;
//        U newG = static_cast<U>(lhs.g) * rhs;
//        U newB = static_cast<U>(lhs.b) * rhs;
//
//        return RGB{
//            .r = ClampValue<T>(newR),
//            .g = ClampValue<T>(newG),
//            .b = ClampValue<T>(newB)
//        };
//    }
//};
//
//template <typename T, typename U>
//constexpr bool operator==(const RGBA<T>& lhs, const RGBA<U>& rhs)
//{
//    auto [r1, r2] = WideCommonCast(lhs.r, rhs.r);
//    auto [g1, g2] = WideCommonCast(lhs.g, rhs.g);
//    auto [b1, b2] = WideCommonCast(lhs.b, rhs.b);
//    auto [a1, a2] = WideCommonCast(lhs.a, rhs.a);
//
//    return r1 == r2 && g1 == g2 && b1 == b2 && a1 == a2;
//}
//
//template <typename T, typename U>
//constexpr RGBA<T> operator+(const RGBA<T>& lhs, const RGBA<U>& rhs)
//{
//    auto [r1, r2] = WideCommonCast(lhs.r, rhs.r);
//    auto [g1, g2] = WideCommonCast(lhs.g, rhs.g);
//    auto [b1, b2] = WideCommonCast(lhs.b, rhs.b);
//    auto [a1, a2] = WideCommonCast(lhs.a, rhs.a);
//
//    
//   auto rRes = static_cast<T>(SafeIntegralAdd(r1, r2));
//   auto gRes = static_cast<T>(SafeIntegralAdd(g1, g2));
//   auto bRes = static_cast<T>(SafeIntegralAdd(b1, b2));
//   auto aRes = static_cast<T>(SafeIntegralAdd(a1, a2)); 
//
//   using Common = safe_common_integral_type_t<T, U>;
//   
//   return RGBA<T>{
//       ClampToLimits<T>(rRes),
//           ClampToLimits<T>(gRes),
//           ClampToLimits<T>(bRes),
//           ClampToLimits<T>(aRes) 
//   };
//}
//
//template <typename T, typename U>
//constexpr RGBA<T> operator-(const RGBA<T>& lhs, const RGBA<U>& rhs)
//{
//    auto [r1, r2] = WideCommonCast(lhs.r, rhs.r);
//    auto [g1, g2] = WideCommonCast(lhs.g, rhs.g);
//    auto [b1, b2] = WideCommonCast(lhs.b, rhs.b);
//    auto [a1, a2] = WideCommonCast(lhs.a, rhs.a);
//
//    auto rRes = SafeIntegralSubtract(r1, r2);
//    auto gRes = SafeIntegralSubtract(g1, g2);
//    auto bRes = SafeIntegralSubtract(b1, b2);
//    auto aRes = SafeIntegralSubtract(a1, a2);
//
//    using Common = safe_common_integral_type_t<T, U>;
//
//    return RGBA<T>{
//        ClampToLimits<T>(rRes),
//        ClampToLimits<T>(gRes),
//        ClampToLimits<T>(bRes),
//        ClampToLimits<T>(aRes)
//    };
//}