#include "EntityStateSystem.h"
#include "../ecs/Ecs.h"
#include "../core/CommonEntityMethods.h"

namespace {

bool IsTransitionViewValid(const StateTransitionView& view)
{
	return view.name != kInvalidHashName && view.fn != nullptr;
}

} // unnamed

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

void EntityStateSystem::HandleTransitionFulfillmentRequest(Entity& entity)
{
	auto& request = entity.GetComponent<StateTransitionFulfillmentRequest>();
		
	// validate request
	if (request.transitionName == kInvalidHashName)
	{
		LOG_WARNING("State transition request did not have a valid transition name");
		return;
	}
	if (request.stateName == kInvalidHashName)
	{
		LOG_WARNING("State transition request did not have a valid state name");
		return;
	}

	auto view = registry_.RegisterOrRetrieveTransition(std::move(request));

	if (IsTransitionViewValid(view))
	{
		// fill the requesting entity's state transition with the view
		auto root = GetRootEntity(entity);
		assert(root.IsValid());

		if (!root.HasComponent<EntityStates>())
		{
			LOG_WARNING("Root entity had not made a request for this transition");
		}
		else
		{
			auto& states = root.GetComponent<EntityStates>();

			auto it = states.table.find(request.stateName);
			if (it == states.table.end())
			{
				LOG_WARNING("Root entity did not have a state matching the state name in request");
				return;
			}

			auto& transitions = it->second.transitions;

			if (transitions.onEnter.name == view.name)
			{
				transitions.onEnter.fn = view.fn;
			}
			if (transitions.onExit.name == view.name)
			{
				transitions.onExit.fn = view.fn;
			}
		}
	}

	if (entity.HasComponent<ComponentDelegate>())
	{
		entity.RemoveComponent<StateTransitionFulfillmentRequest>();
	}		
}


void EntityStateSystem::Update()
{
	auto entities = ECS::GetAllEntitiesWith<StateTransitionFulfillmentRequest>();

	for (auto& entity : entities)
	{
		HandleTransitionFulfillmentRequest(entity);
	}


	/*auto entities = ECS::GetAllEntitiesWith<NeedsUpdate, EntityStates>(
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
	}*/
}
