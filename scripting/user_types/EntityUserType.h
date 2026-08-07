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
	Timer
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
				 "getID", &Entity::GetID
	);
} 