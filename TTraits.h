#pragma once
#include <concepts>
#include <type_traits>

template <typename T, typename...Ts>
static constexpr bool type_in_pack_v = (std::same_as<T, Ts> || ...);

template <typename T, typename...Ts>
concept PackMemberType = type_in_pack_v<T, Ts...>;

template <typename...Ts>
struct pack_types_unique;

template <typename T, typename U, typename...Ts>
struct pack_types_unique<T, U, Ts...>
{
	static constexpr bool value = !(std::same_as<T, U>) && pack_types_unique<Ts...>;
};

template <typename...Ts>
static constexpr bool pack_types_unique_v = pack_types_unique<Ts...>::value;

//template <typename T>
//concept StdVectorType = requires { typename T::value_type; } &&
//	std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;

template <typename T> concept ArithmeticType = std::is_arithmetic_v<T>;

template <typename T, typename U> concept ConvertibleType = std::convertible_to<T, U>;

template <typename T> concept UIntegralType = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T> concept SignedType = std::is_signed_v<T>;

//template <typename T, typename U, typename...Ts>
//struct pack_types_unique<T, U, Ts...>
//{
//	static constexpr bool value = !(std::same_as<T, U>) && pack_types_unique<Ts...>;
//};
