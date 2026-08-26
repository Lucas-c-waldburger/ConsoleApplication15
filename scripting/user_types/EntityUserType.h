#pragma once
#include "../LuaUserType.h"
#include "../../ecs/Ecs.h"
#include "TransformLuaUserTypes.h"
#include "AudioUserType.h"
#include "GameControllerLuaUserTypes.h"
#include "RenderableLuaUserTypes.h"
#include "SpriteAnimationLuaUserTypes.h"
#include "NameUserType.h"
#include "PhysicsUserTypes.h"
#include "TimerComponentUserType.h"
#include "ComponentIdLuaUserType.h"

namespace lua_component_method_detail {

template <typename T> struct lua_component_method_dispatch_table;
template <template <typename...> class TList, typename...Ts>
struct lua_component_method_dispatch_table<TList<Ts...>> {

	template <typename T>
	static sol::object get_component_impl(sol::state_view state, Entity& e)
	{
		return sol::make_object(state, std::ref(e.GetComponent<T>()));
	}
	template <typename T>
	static bool has_component_impl(const Entity& e)
	{
		return e.HasComponent<T>();
	}
	template <typename T>
	static sol::object add_component_impl(sol::state_view state, Entity& e)
	{
		return sol::make_object(state, std::ref(e.AddComponent<T>()));
	}
	template <typename T>
	static void remove_component_impl(Entity& e)
	{
		e.RemoveComponent<T>();
	}

	using GetAddSig = sol::object(*)(sol::state_view, Entity&);
	using HasSig = bool(*)(const Entity&);
	using RemoveSig = void(*)(Entity&);

	static constexpr GetAddSig get_component[] = { &get_component_impl<Ts>... };
	static constexpr HasSig has_component[] = { &has_component_impl<Ts>... };
	static constexpr GetAddSig add_component[] = { &add_component_impl<Ts>... };
	static constexpr RemoveSig remove_component[] = { &remove_component_impl<Ts>... };
};
} // lua_component_method_detail

inline constexpr auto kLuaGetComponentDispatchTable =
lua_component_method_detail::lua_component_method_dispatch_table<LuaComponentTypeList>::get_component;
inline constexpr auto kLuaHasComponentDispatchTable =
lua_component_method_detail::lua_component_method_dispatch_table<LuaComponentTypeList>::has_component;
inline constexpr auto kLuaAddComponentDispatchTable =
lua_component_method_detail::lua_component_method_dispatch_table<LuaComponentTypeList>::add_component;
inline constexpr auto kLuaRemoveComponentDispatchTable =
lua_component_method_detail::lua_component_method_dispatch_table<LuaComponentTypeList>::remove_component;

#define DEF_LUA_COMPONENT_METHODS(cmpType) \
	"get"	 #cmpType, [](Entity& e) -> cmpType& { return e.GetComponent<cmpType>(); }, \
	"has"	 #cmpType, [](const Entity& e) { return e.HasComponent<cmpType>(); }, \
	"add"	 #cmpType, [](Entity& e) -> cmpType& { return e.AddComponent<cmpType>(); }, \
	"remove" #cmpType, [](Entity& e) { return e.RemoveComponent<cmpType>(); }


using EntityLuaUserTypeDependencies = Dependencies<
	Transform, 
	SpriteRenderableComponent, 
	TextRenderableComponent,
	SpriteAnimationComponent, 
	Name, 
	Timer,
	NewAudioRequest,
	GameControllerState,
	ComponentId
>;

DEF_LUA_USERTYPE(Entity, EntityLuaUserTypeDependencies)
{
	lua.def_type(DEF_LUA_COMPONENT_METHODS(Transform),
				 DEF_LUA_COMPONENT_METHODS(SpriteRenderableComponent),
				 DEF_LUA_COMPONENT_METHODS(TextRenderableComponent),
				 DEF_LUA_COMPONENT_METHODS(SpriteAnimationComponent),
				 DEF_LUA_COMPONENT_METHODS(Name),
				 DEF_LUA_COMPONENT_METHODS(Timer),
				 "isValid", &Entity::IsValid,
				 "getId", &Entity::GetID,
				 "getComponent", [](sol::this_state state, Entity& e, ComponentId cmpId) {
					return std::invoke(kLuaGetComponentDispatchTable[cmpId.index], state, e);
				 },
				 "hasComponent", [](const Entity& e, ComponentId cmpId) {
					return std::invoke(kLuaHasComponentDispatchTable[cmpId.index], e);
				 },
				 "addComponent", [](sol::this_state state, Entity& e, ComponentId cmpId) {
					return std::invoke(kLuaAddComponentDispatchTable[cmpId.index], state, e);
				 },
				 "removeComponent", [](Entity& e, ComponentId cmpId) {
					std::invoke(kLuaRemoveComponentDispatchTable[cmpId.index], e);
				 }
	);
} 