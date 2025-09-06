#pragma once
#include "BaseComponent.h"
#include "../core/commonObjects.h"
#include "../state/EntityState.h"

//struct EntityState
//{
//	using StateLinks = std::unordered_set<std::string>;
//
//	struct Transitions
//	{
//		StateTransitionCallback::View onEnter;
//		StateTransitionCallback::View onExit;
//	};
//
//	Transitions transitions;
//	StateLinks stateLinks;
//};

//struct StateHandle 
//{
//	size_t slotIndex = kInvalidIndex;
//	size_t gen = kInvalidIndex;
//
//	bool operator==(const StateHandle&) const = default;
//};

//struct StateNode
//{
//	StateHandle stateHandle;
//
//};

struct EntityStateComponent : BaseComponent<EntityStateComponent>
{
	Requestable<size_t> stateID;
};



