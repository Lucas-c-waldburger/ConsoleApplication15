#pragma once
#include "BaseComponent.h"
#include "util/CallbackUtils.h"
#include "../core/Signal.h"

struct EventCallbacks : BaseComponent<EventCallbacks>
{
	std::unordered_map<uint32_t, std::vector<Handle<EventCallback>>> table;
};

struct SignalTokenStorage : BaseComponent<SignalTokenStorage>
{
	std::vector<SignalToken> signalTokens;
};