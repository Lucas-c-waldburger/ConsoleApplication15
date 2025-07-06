#pragma once
#include <iostream>
#include <optional>
#include "../ecs/EntityT.h"
#include "../core/CommonFunctions.h"

struct BaseCallbackDescriptor
{
	std::string callbackName;
	std::optional<Entity_t> uniqueOwner;

	bool operator==(const BaseCallbackDescriptor& rhs) const
	{
		return callbackName == rhs.callbackName && uniqueOwner == rhs.uniqueOwner;
	}
};

namespace std {
	template <>
	struct hash<BaseCallbackDescriptor> {
		size_t operator()(const BaseCallbackDescriptor& desc) const noexcept {
			size_t hash = 0;
			HashCombine(hash, std::hash<std::string>{}(desc.callbackName));
			HashCombine(hash, std::hash<std::optional<Entity_t>>{}(desc.uniqueOwner));

			return hash;
		}
	};
}

template <typename T>
concept SomeCallbackDescriptor = std::derived_from<T, BaseCallbackDescriptor>;