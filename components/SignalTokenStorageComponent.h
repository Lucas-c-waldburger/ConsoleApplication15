#pragma once
#include <vector>
#include "../core/Signal.h"
#include "BaseComponent.h"

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

	Type type = Type::Event;
	SignalToken token;
};
 
struct SignalTokenStorage : BaseComponent<SignalTokenStorage>
{
	std::vector<EntityCallbackToken> signalTokens;
};