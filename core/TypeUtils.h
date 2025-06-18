#pragma once
#include <concepts>
#include <type_traits>
#include <unordered_set>
#include "commonObjects.h"

template <typename...Ts> struct TypeList 
{
	static constexpr size_t size = sizeof...(Ts);

	template <typename Fn, typename...Args>
	static constexpr decltype(auto) Apply(Fn&& fn, Args&&...args)
	{
		return std::forward<Fn>(fn).template operator()<Ts...>(std::forward<Args>(args)...);
	}

	// for static member functions
	// must wrap static function in class with static method named "Apply"
	template <typename Fn, typename...Args>
	static constexpr decltype(auto) Apply(Args&&...args)
	{
		Fn::template Apply<Ts...>(std::forward<Args>(args)...);
	}
};

/* TYPE PRESENT IN TYPE LIST/PARAMETER PACK */
namespace detail {
template <typename T, typename TList>
struct type_in_list;

template <typename T, typename...Ts>
struct type_in_list<T, TypeList<Ts...>> 
{ 
	static constexpr bool value = (std::same_as<T, Ts> || ...);
};
} // detail

template <typename T, typename TList>
inline constexpr bool type_in_list_v = detail::type_in_list<T, TList>::value;

template <typename T, typename TList>
concept SomeTypeInList = type_in_list_v<T, TList>;

template <typename T, typename...Ts>
static constexpr bool type_in_pack_v = (std::same_as<T, Ts> || ...);

template <typename T, typename...Ts>
concept SomeTypeInPack = type_in_pack_v<T, Ts...>;
/**/

/* TYPES IN PARAMETER PACK UNIQUE */
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
/**/

/* INDEX OF TYPE IN TYPE LIST */
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
/**/

/* CONCAT TYPE LISTS */
namespace detail {
template <typename TL1, typename TL2>
struct concat;

template <typename... Ts, typename... Us>
struct concat<TypeList<Ts...>, TypeList<Us...>> {
	using type = TypeList<Ts..., Us...>;
};

template <typename... Lists>
struct concat_type_lists_impl;

template <typename List>
struct concat_type_lists_impl<List> {
	using type = List;
};

template <typename L1, typename L2, typename... Rest>
struct concat_type_lists_impl<L1, L2, Rest...> {
	using type = typename concat_type_lists_impl<typename concat<L1, L2>::type, Rest...>::type;
};
} // detail

template <typename...TLists>
using concat_type_lists_t = detail::concat_type_lists_impl<TLists...>::type;
/**/

/* TYPE AT TYPE LIST INDEX */
namespace detail {
template <size_t Idx, size_t Counter, typename TList>
struct type_at_index;

template <size_t Idx, size_t Counter>
struct type_at_index<Idx, Counter, TypeList<>> {
	static_assert(Idx < Counter, "Index out of bounds in TypeList.");
	using type = void;
};

template <size_t Idx, size_t Counter, typename T, typename... Ts>
struct type_at_index<Idx, Counter, TypeList<T, Ts...>> {
	using type = std::conditional_t<
		Idx == Counter,
		T,
		typename type_at_index<Idx, Counter + 1, TypeList<Ts...>>::type
	>;
};
} // detail

template <size_t Idx, typename TList>
using type_at_index_t = typename detail::type_at_index<Idx, 0, TList>::type;
/**/

/* IS CONST REFERENCE */
namespace detail {
template <typename T>
struct is_const_reference
{
	static constexpr bool value = std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;
};
} // detail

template <typename T>
inline constexpr bool is_const_reference_v = detail::is_const_reference<T>::value;
/**/

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

