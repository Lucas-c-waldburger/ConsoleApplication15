#pragma once
#include <iostream>
#include <optional>
#include "../ecs/EntityT.h"
#include "../core/CommonFunctions.h"

struct BaseCallbackKey
{
	std::string callbackName;
	std::optional<Entity_t> uniqueOwner;

	friend bool operator==(const BaseCallbackKey& lhs, const BaseCallbackKey& rhs)
	{
		return lhs.callbackName == rhs.callbackName &&
			lhs.uniqueOwner == rhs.uniqueOwner;
	}
};

namespace std {
	template <>
	struct hash<BaseCallbackKey> {
		size_t operator()(const BaseCallbackKey& details) const noexcept {
			size_t hash = 0;
			HashCombine(hash, std::hash<std::string>{}(details.callbackName));
			HashCombine(hash, std::hash<std::optional<Entity_t>>{}(details.uniqueOwner));

			return hash;
		}
	};
}

template <typename T>
concept SomeCallbackKey = std::derived_from<T, BaseCallbackKey>;