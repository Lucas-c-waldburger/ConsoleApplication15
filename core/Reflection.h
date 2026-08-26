#pragma once
#include <boost/pfr.hpp>
#include <concepts>
#include <sol/sol.hpp>
#include <magic_enum/magic_enum.hpp>
//#include "../deps/nlohmann/json.hpp"
#include "TypeInfo.h"
#include "TypeUtils.h"

template <typename T>
concept PfrReflectable = requires {
	boost::pfr::tuple_size_v<std::remove_cvref_t<T>>;
};

template <typename E, size_t...Is>
inline void AutoRegisterEnumImpl(sol::state_view state, std::string_view enumName,
	const std::array<std::pair<E, std::string_view>, sizeof...(Is)>& enumEntries,
	std::index_sequence<Is...>)
{
	auto args = std::tuple_cat(
		std::forward_as_tuple(enumEntries[Is].second, enumEntries[Is].first)...
	);

	std::apply([&](auto&&...args) {
		state.new_enum(enumName, std::forward<decltype(args)>(args)...);
	}, args);
}

template <typename E> requires std::is_enum_v<E>
inline void AutoRegisterEnum(sol::state_view state, std::string_view enumName)
{
	static constexpr auto enumEntries = magic_enum::enum_entries<E>();

	AutoRegisterEnumImpl(state, enumName, enumEntries, std::make_index_sequence<enumEntries.size()>{});
}

//template <typename E> requires std::is_enum_v<E>
//inline bool AutoRegisterEnum(sol::state& state)
//{
//	return AutoRegisterEnum<E>(state, TypeInfo<E>::name);
//}

template <size_t I, typename T>
inline auto AutoMakeSolProperty()
{
	using FieldType = std::remove_cvref_t<
		boost::pfr::tuple_element_t<I, T>
	>;

	return sol::property(
		[](T& obj) -> FieldType&
		{
			return boost::pfr::get<I>(obj);
		},
		[](T& obj, FieldType value)
		{
			boost::pfr::get<I>(obj) = std::move(value);
		}
	);
}

template <typename T, size_t...Is>
inline void AutoRegisterUserTypeImpl(sol::state_view state, std::string_view typeName, 
									 std::index_sequence<Is...>)
{
	auto args = std::tuple_cat(
		std::make_tuple(
			boost::pfr::get_name<Is, T>(),
			AutoMakeSolProperty<Is, T>()
		)...
	);

	std::apply([&](auto&&...args) {
		state.new_usertype<T>(typeName, std::forward<decltype(args)>(args)...);
	}, std::move(args));
}

template <typename T> requires PfrReflectable<T>
inline void AutoRegisterUserType(sol::state_view state, std::string_view typeName)
{
	AutoRegisterUserTypeImpl<T>(state, typeName, std::make_index_sequence<boost::pfr::tuple_size_v<T>>{});
}

//template <typename BasicJson, typename T>
//concept SomeJsonSerializable = requires(BasicJson& j, const T& t) {
//	{ to_json(j, t) } -> std::same_as<void>;
//};
//template <typename BasicJson, typename T>
//concept SomeJsonDeserializable = requires(const BasicJson& j, T& t) {
//	{ from_json(j, t) } -> std::same_as<void>;
//};
//
//template <typename T, size_t...Is> requires PfrReflectable<T>
//consteval bool AllMemberTypesJsonSerializableImpl(std::index_sequence<Is...>)
//{
//	return (SomeJsonSerializable<std::remove_cvref_t<boost::pfr::tuple_element_t<Is, T>>> && ...);
//}
//
//template <typename T, size_t...Is> requires PfrReflectable<T>
//consteval bool AllMemberTypesJsonDeserializableImpl(std::index_sequence<Is...>)
//{
//	return (SomeJsonDeserializable<std::remove_cvref_t<boost::pfr::tuple_element_t<Is, T>>> && ...);
//}
//
//template <typename T> requires PfrReflectable<T>
//consteval bool AllMemberTypesJsonSerializable()
//{
//	return AllMemberTypesJsonSerializableImpl(std::make_index_sequence<boost::pfr::tuple_size_v<T>>{});
//}
//
//template <typename T> requires PfrReflectable<T>
//consteval bool AllMemberTypesJsonDeserializable()
//{
//	return AllMemberTypesJsonDeserializableImpl(std::make_index_sequence<boost::pfr::tuple_size_v<T>>{});
//}
//
//template <typename BasicJson, typename T> requires PfrReflectable<T>
//void to_json(BasicJson& j, const T& obj)
//{
//	boost::pfr::for_each_field_with_name(obj, [&j](std::string_view name, const auto& value) {
//		using MemberType = raw_type_t<decltype(value)>;
//		static_assert(SomeJsonSerializable<MemberType>);
//
//		j[name] = value;
//	});
//}
//
//template <typename BasicJson, typename T> requires PfrReflectable<T>
//void from_json(BasicJson& j, const T& obj)
//{
//	boost::pfr::for_each_field_with_name(obj, [&j](std::string_view name, auto& value) {
//		using MemberType = raw_type_t<decltype(value)>;
//		static_assert(SomeJsonDeserializable<MemberType>);
//
//		value = j.at(name).get<MemberType>();
//	});
//}

//template <typename T> requires PfrReflectable<T>
//inline bool AutoRegisterUserType(sol::state& state)
//{
//	return AutoRegisterUserType<T>(state, TypeInfo<T>::name);
//}