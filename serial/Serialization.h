#pragma once
#include <array>
#include <iostream>
#include "../deps/nlohmann/json.hpp"
#include "../components/ComponentConcepts.h"
#include "../components/ComponentIncludes.h"
#include "../core/TypeUtils.h"

// TODO: how to handle dynamic things like atlas handles, entity ids, etc.

template <typename T>
concept JsonSerializable = requires(nlohmann::json& j, const T& t) {
	to_json(j, t);   
};

template <SomeComponent T>
struct ComponentTypeToName;

template <FixedString Name>
struct ComponentNameToType;

// bi-directional between name and type
template <typename T>
concept HasComponentName = requires {
	std::convertible_to<typename ComponentTypeToName<T>::value, std::string_view>;
	requires std::same_as<
		typename ComponentNameToType<ComponentTypeToName<T>::value>::type, T
	>;
};

#define DEF_COMPONENT_SERIALIZABLE(cmpType, ...)		 \
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cmpType, __VA_ARGS__) \
template <> struct ComponentTypeToName<cmpType> {		 \
	static constexpr std::string_view value = #cmpType;  \
};														 \
template <> struct ComponentNameToType<#cmpType> {		 \
	using type = cmpType;								 \
}		

template <typename T>
concept JsonSerializableComponent = JsonSerializable<T> && HasComponentName<T>;

namespace detail {
template <typename T>
struct some_json_serializable_component_pred :
	std::bool_constant<JsonSerializableComponent<T>> {
};
} // detail

using SerializableComponentTypeList = 
	filter_types_t<ComponentTypeList, detail::some_json_serializable_component_pred>;


/* COMPONENT DESERIALIZATION TARGET */
template <JsonSerializableComponent T>
using ComponentDeserializationTargetWrapper = Result<std::optional<T>>;
	
using ComponentDeserializationTarget = as_tuple_t<SerializableComponentTypeList,
												  ComponentDeserializationTargetWrapper>;

namespace detail {
template <typename CmpList>
struct make_component_deserialization_target_impl;

template <typename...Ts> requires 
	std::same_as<TypeList<Ts...>, SerializableComponentTypeList>
struct make_component_deserialization_target_impl<TypeList<Ts...>>
{
	static ComponentDeserializationTarget Call()
	{
		return std::make_tuple(Result<std::optional<Ts>>{}, ...);
	}
};
} // detail

inline ComponentDeserializationTarget MakeComponentDeserializationTarget()
{
	return detail::make_component_deserialization_target_impl<
		SerializableComponentTypeList>::Call();
}