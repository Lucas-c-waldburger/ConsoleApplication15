#include "StateTransitionCallback.h"
#include "../ecs/Ecs.h"

//auto StateTransitionCallback::WrapLua(TypedLuaFunction<Void(Entity&)> luaFn)
//{
//	return [fn = std::move(luaFn)](Entity& entity) -> void {
//		LOG_IF_ERROR(fn(entity));
//	};
//}
//
//StateTransitionCallback::StateTransitionCallback(std::string_view name, 
//												 const TypedLuaFunction<Void(Entity&)>& luaFn)
// : ICallbackSource(name, WrapLua(luaFn)) {}
