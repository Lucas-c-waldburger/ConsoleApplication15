#pragma once
#include "../LuaUserType.h"
#include "../../components/SpriteAnimationsComponent.h"


DEF_LUA_USERTYPE(NeedsAnimationUpdate) {
	lua.def_type();
}
  
DEF_LUA_USERTYPE(SpriteSeriesIndex) {
	lua.def_type("current", &SpriteSeriesIndex::current,
				 "max", &SpriteSeriesIndex::max,
				 "increment", [](SpriteSeriesIndex& ssi) { ++ssi; },
				 "decrement", [](SpriteSeriesIndex& ssi) { --ssi; });
}

DEF_LUA_USERTYPE(SpriteAnimationComponent, Dependencies<SpriteSeriesIndex>) {
	lua.def_type("spriteSeriesName", &SpriteAnimationComponent::spriteSeriesName,
				 "index", &SpriteAnimationComponent::index,
				 sol::meta_function::equal_to, &SpriteAnimationComponent::operator==);
}