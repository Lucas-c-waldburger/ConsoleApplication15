#pragma once
#include <vector>
#include "../core/Signal.h"
#include "ComponentConcepts.h"

struct EntityCallbackToken
{
	enum class Type
	{
		Event,
		Script
	};

	EntityCallbackToken() = default;
	EntityCallbackToken(SignalToken&& tk) : type(Type::Event), token(std::move(tk)) {}
	EntityCallbackToken(SignalToken&& tk, Type ty) : type(ty), token(std::move(tk)) {}
	~EntityCallbackToken() = default;
	EntityCallbackToken(const EntityCallbackToken&) = delete;
	EntityCallbackToken& operator=(const EntityCallbackToken&) = delete;
	EntityCallbackToken(EntityCallbackToken&&) noexcept = default;
	EntityCallbackToken& operator=(EntityCallbackToken&&) noexcept = default;

	bool operator==(const EntityCallbackToken&) const = default;

	Type type = Type::Event;
	SignalToken token;
};
 
struct SignalTokenStorage
{
	std::vector<EntityCallbackToken> signalTokens;

	bool operator==(const SignalTokenStorage&) const = default;
};