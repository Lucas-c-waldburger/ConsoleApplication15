//#pragma once
//#include "../LuaUserType.h"
//#include "../../components/ComponentIncludes.h"
//#include "../../ecs/EntityConcepts.h"
//
//template <typename T> struct LuaUserTypePred :
//	std::bool_constant<(SomeLuaUserType<T> && public_mutable_component_v<T>)> {};
//
//using LuaComponentTypeList = filter_types_t<ComponentTypeList, LuaUserTypePred>;
//
//DEF_LUA_USERTYPE(ComponentId) { lua.def_type(); }