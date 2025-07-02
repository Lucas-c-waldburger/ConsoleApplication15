#pragma once
#include "../LuaTypesRegistry.h"
#include "../../components/SpriteAnimationsComponent.h"
#include "../../sprite/SpriteAnimationSeriesMap.h"

template <> inline void RegisterLuaUserType<SpriteAnimationSeries>(sol::state& lua)
{
	if (!lua["SpriteAnimationSeries"].valid())
	{
		lua.new_usertype<SpriteAnimationSeries>("SpriteAnimationSeries",
			"sourceAtlas", &SpriteAnimationSeries::sourceAtlas,
			"sourcePlots", &SpriteAnimationSeries::spritePlots,
			"index", &SpriteAnimationSeries::index);
	}
}

template <> inline void RegisterLuaUserType<SpriteAnimationSeriesMap>(sol::state& lua)
{
	if (!lua["SpriteAnimationSeriesMap"].valid())
	{
		lua.new_usertype<SpriteAnimationSeriesMap>("SpriteAnimationSeriesMap",
			"NextInSeries", &SpriteAnimationSeriesMap::NextInSeries,
			"SetCurrentIndex", &SpriteAnimationSeriesMap::SetCurrentIndex);
	}
}

template <> inline void RegisterLuaUserType<SpriteAnimations>(sol::state& lua)
{
	if (!lua["SpriteAnimations"].valid())
	{
		lua.new_usertype<SpriteAnimations>("SpriteAnimations", "map", &SpriteAnimations::map);
	}
}