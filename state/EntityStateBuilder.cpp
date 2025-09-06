//#include "EntityStateBuilder.h"
//
//Result<EntityStateBuilder> EntityStateBuilder::GetInstance(Entity entity, std::string_view stateName)
//{
//	if (!entity.IsValid())
//	{
//		return MAKE_ERROR("Entity was invalid");
//	}
//
//	if (!entity.HasComponent<EntityStates>())
//	{
//		return MAKE_ERROR("Entity did not have an EntityStates component");
//	}
//
//	if (entity.GetComponent<EntityStates>().table.contains(stateName))
//	{
//		return MAKE_ERROR("Entity already had a state with the given state name");
//	}
//
//	return EntityStateBuilder{ entity, stateName };
//}
//
//void EntityStateBuilder::Finalize()
//{
//	auto& states = entity_.GetComponent<EntityStates>();
//
//	states.table[stateName_] = std::move(state_);
//}
//
////StateTransitionCallback EntityStateBuilder::WrapLuaTransition(TypedLuaFunction<Void(Entity&)>&& luaFn)
////{
////	return [fn = std::move(luaFn)](Entity& entity) -> void {
////		auto result = fn(entity);
////		if (!result.Success())
////		{
////			LOG_ERROR(result.GetError());
////		}
////	};
////}
