#pragma once
#include "BaseComponent.h"
#include "../callbacks/StateTransitionCallback.h"

struct EntityState
{
	using StateLinks = std::unordered_set<HashName, HashNameHash, HashNameEq>;

	struct Transitions
	{
		StateTransitionView onEnter;
		StateTransitionView onExit;
	};

	Transitions transitions;
	StateLinks stateLinks;
};

struct EntityStates : BaseComponent<EntityStates>
{
	HashNameMap<EntityState> table;
	HashName current;
};
