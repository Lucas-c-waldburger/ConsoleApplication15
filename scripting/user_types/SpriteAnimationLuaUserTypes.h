#pragma once
#include "../LuaTypesRegistry.h"
#include "../../components/SpriteAnimationsComponent.h"
#include "../../sprite/SpriteAnimationSeriesMap.h"

template <> static void RegisterLuaUserType<SpriteAnimationSeries>(sol::state& lua)
{
	if (!lua["SpriteAnimationSeries"].valid())
	{
		lua.new_usertype<SpriteAnimationSeries>("SpriteAnimationSeries",
			"sourceAtlas", &SpriteAnimationSeries::sourceAtlas,
			"sourcePlots", &SpriteAnimationSeries::spritePlots,
			"index", &SpriteAnimationSeries::index);
	}
}

template <> static void RegisterLuaUserType<SpriteAnimationSeriesMap>(sol::state& lua)
{
	if (!lua["SpriteAnimationSeriesMap"].valid())
	{
		lua.new_usertype<SpriteAnimationSeriesMap>("SpriteAnimationSeriesMap",
			"Next", &SpriteAnimationSeriesMap::Next);
		//"GetCurrent", 
		//sol::overload(
		//	static_cast<SpriteAnimationSeries*(SpriteAnimationSeriesMap::*)()>
		//		(&SpriteAnimationSeriesMap::GetCurrent),
		//	static_cast<const SpriteAnimationSeries*(SpriteAnimationSeriesMap::*)() const>
		//		(&SpriteAnimationSeriesMap::GetCurrent)
		//));
	}
}

template <> static void RegisterLuaUserType<SpriteAnimations>(sol::state& lua)
{
	if (!lua["SpriteAnimations"].valid())
	{
		lua.new_usertype<SpriteAnimations>("SpriteAnimations",
			"map", &SpriteAnimations::map,
			"dirty", &SpriteAnimations::dirty);
	}
}