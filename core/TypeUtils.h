#pragma once
#include <concepts>
#include <type_traits>
#include <unordered_set>
#include "commonObjects.h"

template <typename...Ts> struct TypeList 
{
	static constexpr size_t size = sizeof...(Ts);
};

namespace detail {
template <typename T, typename TList>
struct type_in_list;

template <typename T, typename...Ts>
struct type_in_list<T, TypeList<Ts...>> { 
	static constexpr bool value = (std::same_as<T, Ts> || ...);
};
}

template <typename T, typename TList>
static constexpr bool type_in_list_v = detail::type_in_list<T, TList>::value;

template <typename T, typename...Ts>
static constexpr bool type_in_pack_v = (std::same_as<T, Ts> || ...);

template <typename T, typename...Ts>
concept PackMemberType = type_in_pack_v<T, Ts...>;


namespace detail {
template <typename...Ts>
struct pack_types_unique : std::true_type {};

template <typename T, typename U, typename...Ts>
struct pack_types_unique<T, U, Ts...>
{
	static constexpr bool value = !(std::same_as<T, U>) && pack_types_unique<Ts...>::value;
};
} // detail

template <typename...Ts>
static constexpr bool pack_types_unique_v = detail::pack_types_unique<Ts...>::value;


namespace detail {
template <typename T, typename TList>
struct index_of;

template <typename T, typename...Ts>
struct index_of<T, TypeList<T, Ts...>> : std::integral_constant<size_t, 0> {};

template <typename T, typename U, typename...Ts>
struct index_of<T, TypeList<U, Ts...>>
	: std::integral_constant<size_t, 1 + index_of<T, TypeList<Ts...>>::value> {};
} // detail

template <typename T, typename TList>
inline constexpr size_t index_of_v = detail::index_of<T, TList>::value;


namespace detail {
template <typename, typename> struct concat_type_lists;
template <typename... Ts1, typename... Ts2>
struct concat_type_lists<TypeList<Ts1...>, TypeList<Ts2...>>
{
	using type = TypeList<Ts1..., Ts2...>;
};
} // detail

template <typename TList1, typename TList2>
using concat_type_lists_t = typename detail::concat_type_lists<TList1, TList2>::type;

// tuple index
//namespace detail {
//template <typename T, typename Tup>
//struct tuple_index;
//
//template <typename T, typename...Ts>
//struct tuple_index<T, std::tuple<T, Ts...>> : std::integral_constant<size_t, 0> {};
//
//template <typename T, typename U, typename...Ts>
//struct tuple_index<T, std::tuple<U, Ts...>>
//	: std::integral_constant<size_t, 1 + tuple_index<T, std::tuple<Ts...>>::value> {};
//}
//
//template <typename T, typename Tup>
//inline constexpr size_t tuple_index_v = detail::tuple_index<T, Tup>::value;



template <typename T> concept ArithmeticType = std::is_arithmetic_v<T>;

template <typename T, typename U> concept ConvertibleType = std::convertible_to<T, U>;

template <typename T> concept UIntegralType = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T> concept SignedType = std::is_signed_v<T>;

template <typename T> 
static constexpr bool is_Void_v = std::is_same_v<std::remove_cvref_t<T>, Void>;

template <typename T>
concept HasBooleanNotOperator = requires(T t) {
	{ !t } -> std::convertible_to<bool>;
};

template <typename T>
concept Hashable = requires(T t) {
	{ std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept UseableInUnorderedSet = requires { typename std::unordered_set<T>; };

