#pragma once
#include "../deps/nlohmann/json.hpp"
#include "../components/ComponentConcepts.h"
#include "../core/commonObjects.h"

template <typename T>
concept JsonSerializable = requires(nlohmann::json & j, const T & t) {
	to_json(std::declval<nlohmann::json&>(),
		std::declval<const T&>());
};

template <SomeComponent T>
struct ComponentName;

template <SomeComponent T>
struct NeedsExtraDeserializing : std::false_type {};

template <SomeComponent T>
inline constexpr bool needs_extra_deserializing_v = NeedsExtraDeserializing::value;

// bi-directional between name and type
template <typename T>
concept HasComponentName = requires {
	{ ComponentName<T>::value } -> std::convertible_to<std::string_view>;
};


#define DEF_SERIALIZABLE_EMPTY(type) \
template <typename BasicJson> inline void to_json(BasicJson&, const type&) {} \
template <typename BasicJson> inline void from_json(const BasicJson&, type&) {} 