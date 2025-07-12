#include "EntityStateSystem.h"
#include "../ecs/Ecs.h"

StateTransitionView EntityStateSystem::TransitionRegistry::GetTransition(std::string_view transitionName)
{
	return impl_.GetTransition(transitionName);
}

StateTransitionView EntityStateSystem::TransitionRegistry::GetTransition(HashName transitionHashName)
{
	return impl_.GetTransition(transitionHashName);
}

bool EntityStateSystem::TransitionRegistry::EraseTransition(std::string_view transitionName)
{
	if (!impl_.EraseTransition(transitionName))
	{
		return false;
	}

	auto entities = ECS::GetAllEntitiesWith<EntityStates>();

	for (auto& entity : entities)
	{
		auto& states = entity.GetComponent<EntityStates>();

		for (auto& [_, state] : states.table)
		{
			if (state.transitions.onEnter.name == transitionName)
			{
				state.transitions.onEnter.name = kInvalidHashName;
				state.transitions.onEnter.fn = nullptr;
			}
			if (state.transitions.onExit.name == transitionName)
			{
				state.transitions.onExit.name = kInvalidHashName;
				state.transitions.onExit.fn = nullptr;
			}
		}
	}

	return true;
}

void EntityStateSystem::Update()
{
	auto entities = ECS::GetAllEntitiesWith<NeedsUpdate, EntityStates>(
		[](const NeedsUpdate& update, const EntityStates&) {
			return update.components & EntityStates::componentBit;
		});

	for (auto& entity : entities)
	{
		auto& states = entity.GetComponent<EntityStates>();

		for (auto& [_, state] : states.table)
		{
			FulfillTransitionRequest(state.transitions.onEnter);
			FulfillTransitionRequest(state.transitions.onExit);
		}

		auto& update = entity.GetComponent<NeedsUpdate>();

		update.components &= ~(EntityStates::componentBit);
	}
}

void EntityStateSystem::FulfillTransitionRequest(StateTransitionView& transition)
{
	if (transition.fn || transition.name == kInvalidHashName)
	{
		return;
	}

	transition = registry_.GetTransition(transition.name);

	if (!transition.fn)
	{
		transition.name = kInvalidHashName;
	}
}
