#pragma once
#include "commonObjects.h"
#include <concepts>
#include <type_traits>
#include <unordered_set>

template <typename...Ts> struct TypeList 
{
	static constexpr size_t size = sizeof...(Ts);

	template <typename T>
	static constexpr bool contains = (std::same_as<T, Ts> || ...);

	template <typename Fn, typename...Args>
	static constexpr decltype(auto) Apply(Fn&& fn, Args&&...args)
	{
		return std::forward<Fn>(fn).template operator()<Ts...>(std::forward<Args>(args)...);
	}

	template <typename Fn>
	static constexpr void ForEachType(Fn&& fn)
	{
		((fn.template operator()<Ts>()), ...);
	}

	// for static member functions
	// must wrap static function in class with static method named "Apply"
	template <typename Fn, typename...Args>
	static constexpr decltype(auto) Apply(Args&&...args)
	{
		Fn::template Apply<Ts...>(std::forward<Args>(args)...);
	}
};

/* IS TYPE LIST */
namespace detail {
template <typename T>
struct is_type_list : std::false_type {};

template <typename...Ts>
struct is_type_list<TypeList<Ts...>> : std::true_type {};
}

template <typename T>
inline constexpr bool is_type_list_v = detail::is_type_list<T>::value;

/* TYPE PRESENT IN TYPE LIST/PARAMETER PACK/TUPLE */
namespace detail {
template <typename T, typename TList>
struct type_in_list;

template <typename T, typename...Ts>
struct type_in_list<T, TypeList<Ts...>> 
{ 
	static constexpr bool value = (std::same_as<T, Ts> || ...);
};

template <typename T, typename Tuple>
struct type_in_tuple;

template <typename T, typename...Ts>
struct type_in_tuple<T, std::tuple<Ts...>>
{
	static constexpr bool value = (std::same_as<T, Ts> || ...);
};
} // detail

template <typename T, typename TList>
inline constexpr bool type_in_list_v = detail::type_in_list<T, TList>::value;

template <typename T, typename TList>
concept SomeTypeInList = type_in_list_v<T, TList>;

template <typename T, typename...Ts>
inline constexpr bool type_in_pack_v = (std::same_as<T, Ts> || ...);

template <typename T, typename...Ts>
concept SomeTypeInPack = type_in_pack_v<T, Ts...>;

template <typename...Ts, typename...Us>
concept AllTypesInPack = (type_in_pack_v<Ts, Us...> && ...);

template <typename T, typename Tuple>
inline constexpr bool type_in_tuple_v = detail::type_in_tuple<T, Tuple>::value;

template <typename T, typename Tuple>
concept SomeTypeInTuple = type_in_tuple_v<T, Tuple>;
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

/* UNIQUE TYPE LIST */
namespace detail {
template <typename TList>
struct unique_type_list : std::false_type {};

template <typename...Ts>
struct unique_type_list<TypeList<Ts...>>
{
	static constexpr bool value = pack_types_unique_v<Ts...>;
};
} // detail

template <typename TList>
static constexpr bool unique_type_list_v = detail::unique_type_list<TList>::value;
/**/

/* INDEX OF TYPE IN TUPLE-LIKE */
namespace detail {

template <typename T, typename TupLike>
struct index_of;

template <typename T, template <typename...> class TupLike>
struct index_of<T, TupLike<>> {
	static_assert(sizeof(T) == 0, "Type not found");
};

template <typename T, template <typename...> class TupLike, typename...Ts>
struct index_of<T, TupLike<T, Ts...>> : std::integral_constant<size_t, 0> {};

template <typename T, template <typename...> class TupLike, typename U, typename...Ts>
struct index_of<T, TupLike<U, Ts...>>
	: std::integral_constant<size_t, 1 + index_of<T, TupLike<Ts...>>::value> {
};

//template <typename T, typename TList>
//struct index_of;
//
//template <typename T, typename...Ts>
//struct index_of<T, TypeList<T, Ts...>> : std::integral_constant<size_t, 0> {};
//
//template <typename T, typename U, typename...Ts>
//struct index_of<T, TypeList<U, Ts...>>
//	: std::integral_constant<size_t, 1 + index_of<T, TypeList<Ts...>>::value> {};
} // detail

template <typename T, typename TupLike>
inline constexpr size_t index_of_v = detail::index_of<T, TupLike>::value;
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

/* TRANSFORM / FILTER TUPLE */
/* PREPEND */
namespace detail {
template <typename T, typename TList>
struct prepend_type;

template <typename T, typename...Ts>
struct prepend_type<T, TypeList<Ts...>> {
	using type = TypeList<T, Ts...>;
};
} // detail

template <typename T, typename TList>
using prepend_type_t = detail::prepend_type<T, TList>::type;

/* FILTER */
namespace detail {
template <typename List, template <typename> class Pred>
struct filter_types;

template <template <typename> class Pred>
struct filter_types<TypeList<>, Pred> {
	using type = TypeList<>;
};

template <typename T, typename... Ts, template <typename> class Pred>
struct filter_types<TypeList<T, Ts...>, Pred> {
private:
	using tail = typename filter_types<TypeList<Ts...>, Pred>::type;

public:
	using type = std::conditional_t<
		Pred<T>::value, prepend_type_t<T, tail>,
		tail
	>;
};
} // detail

template <typename List, template <typename> class Pred>
using filter_types_t = detail::filter_types<List, Pred>::type;

/* AS TUPLE */
namespace detail {
//template <typename List>
//struct as_tuple;
//
//template <template <typename...> class List, typename...Ts>
//struct as_tuple<List<Ts...>> {
//	using type = std::tuple<Ts...>;
//};

template <typename List>
struct as_tuple;

template <template <typename...> class List, typename...Ts>
struct as_tuple<List<Ts...>> {
	template <template <typename> class Wrap>
	struct inner {
		using type = std::tuple<Wrap<Ts>...>;
	};
};

} // detail

template <typename List, template <typename> class Wrap = std::type_identity_t>
using as_tuple_t = 
	typename detail::as_tuple<List>::template inner<Wrap>::type;

/* TYPE AT TUPLE-LIKE INDEX */
namespace detail {
//template <size_t Idx, typename Tup>
//struct type_at_index;
//
//template <template <typename...> class TupLike, typename T, typename... Ts>
//struct type_at_index<0, TupLike<T, Ts...>> {
//	using type = T;
//};
//
//template <size_t Idx, template <typename...> class TupLike, typename T, typename... Ts>
//struct type_at_index<Idx, TupLike<T, Ts...>> {
//	using type = typename type_at<Idx - 1, TupLike<Ts...>>::type;
//};


template <size_t Idx, size_t Counter, typename TList>
struct type_at_index;

template <size_t Idx, size_t Counter>
struct type_at_index<Idx, Counter, TypeList<>> {
	//static_assert(Idx < Counter, "Index out of bounds in TypeList.");
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

/* RAW TYPE */
template <typename T>
using raw_type_t = std::remove_pointer_t<std::remove_cvref_t<T>>;

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

// HAS ARITHMETIC OPERATOR CONCEPTS
template <typename T, typename U>
concept HasPlusOperatorFor = requires(const T& t, const U& u) {
	{ t + u } -> std::same_as<T>;
};
template <typename T, typename U>
concept HasMinusOperatorFor = requires(const T& t, const U& u) {
	{ t - u } -> std::same_as<T>;
};
template <typename T, typename U>
concept HasMultOperatorFor = requires(const T& t, const U& u) {
	{ t * u } -> std::same_as<T>;
};
//