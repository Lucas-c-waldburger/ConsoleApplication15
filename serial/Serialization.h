#pragma once
#include <array>
#include <iostream>
#include "user_types/ComponentJsonUserType.h"
#include "../core/TypeUtils.h"

// TODO: how to handle dynamic things like atlas handles, entity ids, etc.	

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

static_assert(SerializableComponentTypeList::size > 0);


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
		return std::make_tuple(Result<std::optional<Ts>>{std::optional<Ts>{}}...);
	}
};
} // detail

inline ComponentDeserializationTarget MakeComponentDeserializationTarget()
{
	return detail::make_component_deserialization_target_impl<
		SerializableComponentTypeList>::Call();
}
