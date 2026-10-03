#pragma once
#include "../LuaStateManager.h"
#include "../../components/ComponentTypeList.h"
#include "../../ecs/EntityConcepts.h"
#include "../../core/TypeUtils.h"

template <typename T>
struct register_lua_usertype;

#define DEF_REGISTER_LUA_USERTYPE(type, nm, ...) \
template <> struct register_lua_usertype<type> { \
	static void call(LuaStateManager& m) { \
		m.NewUserType<type>(nm, __VA_ARGS__); \
	} \
	static constexpr std::string_view name = nm; \
}

#define DEF_REGISTER_LUA_ENUM(type, nm) \
template <> struct register_lua_usertype<type> { \
	static void call(LuaStateManager& m) { \
		m.AutoRegister<type>(nm); \
	} \
	static constexpr std::string_view name = nm; \
}