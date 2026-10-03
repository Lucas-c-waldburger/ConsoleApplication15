#pragma once
#include "UserTypesCommon.h"
#include "../../components/SpriteAnimationsComponent.h"


DEF_REGISTER_LUA_USERTYPE(NeedsAnimationUpdate, "NeedsAnimationUpdate");

DEF_REGISTER_LUA_USERTYPE(SpriteSeriesIndex, "SpriteSeriesIndex",
	"current", &SpriteSeriesIndex::current,
	"max", &SpriteSeriesIndex::max,
	"increment", [](SpriteSeriesIndex& ssi) { ++ssi; },
	"decrement", [](SpriteSeriesIndex& ssi) { --ssi; });

DEF_REGISTER_LUA_USERTYPE(SpriteAnimationComponent, "SpriteAnimationComponent", 
	"spriteSeriesName", &SpriteAnimationComponent::spriteSeriesName,
	"index", &SpriteAnimationComponent::index,
	sol::meta_function::equal_to, &SpriteAnimationComponent::operator==);
