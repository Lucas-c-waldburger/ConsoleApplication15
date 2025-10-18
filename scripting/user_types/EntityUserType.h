#pragma once
#include "../LuaUserType.h"
#include "../../ecs/Ecs.h"

#define COMPONENT_METHODS(cmpType) \
	"GetComponent"	 #cmpType, [](Entity& e) -> cmpType& { return e.GetComponent<cmpType>(); }, \
	"HasComponent"	 #cmpType, [](const Entity& e) { return e.HasComponent<cmpType>(); }, \
	"AddComponent"	 #cmpType, [](Entity& e) -> cmpType& { return e.AddComponent<cmpType>(); }, \
	"RemoveComponent"#cmpType, [](Entity& e) { return e.RemoveComponent<cmpType>(); }
 
DEF_LUA_USERTYPE(Entity, Dependencies<Transform>)
{
	lua.def_type("IsValid", &Entity::IsValid,
				 "GetID", &Entity::GetID,
				 COMPONENT_METHODS(Transform)
	);
}