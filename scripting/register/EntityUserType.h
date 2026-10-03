#pragma once
#include "UserTypesCommon.h"
#include "../../ecs/Ecs.h"
#include "UserTypeIncludesInternal.h"

namespace component_method_dispatch_table_detail {

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

template <typename T>
concept HasLuaUserTypeRegistration = requires(LuaStateManager & m) {
	{ register_lua_usertype<T>::call(m) } -> std::same_as<void>;
	std::same_as<
		std::remove_cvref_t<decltype(register_lua_usertype<T>::name)>,
		std::string_view
	>;
};

template <typename T> struct ComponentLuaUserTypePred :
	std::bool_constant<(HasLuaUserTypeRegistration<T> && public_mutable_component_v<T>)> {};

using DispatchTables = lua_component_method_dispatch_table<
	filter_types_t<ComponentTypeList, ComponentLuaUserTypePred>>;

} // component_method_dispatch_table_detail

using ComponentMethodDispatchTables = component_method_dispatch_table_detail::DispatchTables;

DEF_REGISTER_LUA_USERTYPE(Entity, "Entity",
	"isValid", &Entity::IsValid,
	"getId", &Entity::GetID,
	"getComponent", [](sol::this_state state, Entity& e, ComponentId cmpId) {
		return ComponentMethodDispatchTables::get_component[cmpId.index](state, e);
	},
	"hasComponent", [](const Entity& e, ComponentId cmpId) {
		return ComponentMethodDispatchTables::has_component[cmpId.index](e);
	},
	"addComponent", [](sol::this_state state, Entity& e, ComponentId cmpId) {
		return ComponentMethodDispatchTables::add_component[cmpId.index](state, e);
	},
	"removeComponent", [](Entity& e, ComponentId cmpId) {
		ComponentMethodDispatchTables::remove_component[cmpId.index](e); });