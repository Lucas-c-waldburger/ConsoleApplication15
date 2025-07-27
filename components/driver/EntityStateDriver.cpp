#include "EntityStateDriver.h"

//Result<Void> EntityStateDriver::AddState(std::string_view stateName, 
//										 TransitionNamePair transitionNames, 
//										 std::initializer_list<std::string_view> links)
//{
//	auto& states = GetComponent<EntityStates>();
//	HashName stateNameHash{ stateName };
//
//	if (states.table.contains(stateNameHash))
//	{
//		return MAKE_ERROR_FMT("duplicate state name '{}' in entity's state table", stateName);
//	}
//
//	auto& newState = states.table[stateNameHash];
//
//	newState.transitions.onEnter.name = transitionNames.onEnterName;
//	newState.transitions.onExit.name = transitionNames.onExitName;
//	newState.stateLinks = { links.begin(), links.end() };
//
//	if (states.table.size() == 1)
//	{
//		states.current = stateNameHash;
//	}
//
//	auto& update = GetEntity().AddComponent<NeedsUpdate>();
//
//	update.components |= EntityStates::componentBit;
//
//	return Void{};
//}

Result<EntityStateBuilder> EntityStateDriver::GetStateBuilder(std::string_view stateName)
{
	return EntityStateBuilder::GetInstance(GetEntity(), stateName);
}

bool EntityStateDriver::EraseState(std::string_view stateName)
{
	auto& states = GetComponent<EntityStates>();
	HashName stateNameHash{ stateName };

	bool erased = states.table.erase(stateNameHash);
	if (!erased)
	{
		return false;
	}

	for (auto& [_, states] : states.table)
	{
		states.stateLinks.erase(stateNameHash);
	}

	if (states.current == stateNameHash)
	{
		states.current = kInvalidHashName;
	}

	return true;
}

Result<Void> EntityStateDriver::ChangeState(std::string_view nextStateName)
{
	auto& states = GetComponent<EntityStates>();
	HashName nextStateNameHash{ nextStateName };

	auto it = states.table.find(nextStateNameHash);
	if (it == states.table.end())
	{
		return MAKE_ERROR_FMT("State name '{}' not found in entity's state table", nextStateName);
	}

	if (auto* currentState = GetCurrentEntityState(states))
	{
		if (!currentState->stateLinks.contains(nextStateNameHash))
		{
			return MAKE_ERROR_FMT("State name '{}' not linked to entity's current state",
				nextStateName);
		}

		if (currentState->transitions.onExit.fn)
		{
			currentState->transitions.onExit.fn(GetEntity());
		}
	}

	states.current = nextStateNameHash;

	auto& newState = states.table[nextStateNameHash];

	if (newState.transitions.onEnter.fn)
	{
		newState.transitions.onEnter.fn(GetEntity());
	}

	return Void{};
}

bool EntityStateDriver::LinkedToCurrentState(std::string_view stateLink) const
{
	const auto& states = GetComponent<EntityStates>();
	if (states.current == kInvalidHashName)
	{
		return false;
	}

	auto it = states.table.find(states.current);
	if (it == states.table.end())
	{
		return false;
	}

	return it->second.stateLinks.contains(stateLink);
}

EntityState* EntityStateDriver::GetCurrentEntityState(EntityStates& states)
{
	if (states.current == kInvalidHashName)
	{
		return nullptr;
	}

	auto it = states.table.find(states.current);

	return (it != states.table.end()) ? &it->second : nullptr;
}

