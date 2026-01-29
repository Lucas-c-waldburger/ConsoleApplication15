#pragma once
#include <vector>
#include "../core/Signal.h"
#include "BaseComponent.h"

struct SignalTokenStorage : BaseComponent<SignalTokenStorage>
{
	std::vector<SignalToken> signalTokens;
};