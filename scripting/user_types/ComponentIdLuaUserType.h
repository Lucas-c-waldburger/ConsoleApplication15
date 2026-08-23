#pragma once
#include "../LuaUserType.h"
#include "../../components/ComponentIncludes.h"

enum class ComponentId : ComponentSignature
{
	Transform = Transform::componentBit,
	SpriteRenderableComponent = SpriteRenderableComponent::componentBit,
	TextRenderableComponent = TextRenderableComponent::componentBit,
	SpriteAnimationComponent = SpriteAnimationComponent::componentBit,
	Name = Name::componentBit
};

#define DEF_LUA_COMPONENT_ID_FIELD(cmpType) #cmpType, ComponentId::##cmpType

DEF_LUA_USERTYPE(ComponentId) {
	lua.def_enum(
		DEF_LUA_COMPONENT_ID_FIELD(Transform),
		DEF_LUA_COMPONENT_ID_FIELD(SpriteRenderableComponent),
		DEF_LUA_COMPONENT_ID_FIELD(TextRenderableComponent),
		DEF_LUA_COMPONENT_ID_FIELD(SpriteAnimationComponent),
		DEF_LUA_COMPONENT_ID_FIELD(Name)
	);
}