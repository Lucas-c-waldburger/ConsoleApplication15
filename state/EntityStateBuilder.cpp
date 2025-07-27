#include "EntityStateBuilder.h"

Result<EntityStateBuilder> EntityStateBuilder::GetInstance(Entity entity, std::string_view stateName)
{
	if (!entity.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}

	if (!entity.HasComponent<EntityStates>())
	{
		return MAKE_ERROR("Entity did not have an EntityStates component");
	}

	if (entity.GetRelations().IsChild())
	{
		return MAKE_ERROR("Entity was a child");
	}

	auto& states = entity.GetComponent<EntityStates>();
	if (states.table.contains(stateName))
	{
		return MAKE_ERROR("Entity already had a state with the given state name");
	}

	return EntityStateBuilder{ entity, stateName };
}

void EntityStateBuilder::Build()
{
	auto relations = entity_.GetRelations();
	auto& states = entity_.GetComponent<EntityStates>();

	auto& newState = states.table[stateName_];
	newState.stateLinks = std::move(stateLinks_);

	if (onEnterRequest_.transitionName != kInvalidHashName)
	{
		newState.transitions.onEnter.name = onEnterRequest_.transitionName;
		relations.AddComponentDelegate(std::move(onEnterRequest_));
	}

	if (onExitRequest_.transitionName != kInvalidHashName)
	{
		newState.transitions.onExit.name = onExitRequest_.transitionName;
		relations.AddComponentDelegate(std::move(onExitRequest_));
	}
}

StateTransitionCallback EntityStateBuilder::WrapLuaTransition(TypedLuaFunction<Void(Entity&)>&& luaFn)
{
	return [fn = std::move(luaFn)](Entity& entity) -> void {
		auto result = fn(entity);
		if (!result.Success())
		{
			LOG_ERROR(result.GetError());
		}
	};
}
