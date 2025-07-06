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

template <> inline void RegisterLuaUserType<SpriteAnimationsTable>(sol::state& lua)
{
	if (!lua["SpriteAnimationsTable"].valid())
	{
		lua.new_usertype<SpriteAnimationsTable>("SpriteAnimationsTable",
			"NextInSeries", &SpriteAnimationsTable::NextInSeries,
			"SetCurrentIndex", &SpriteAnimationsTable::SetCurrentIndex);
	}
}

template <> inline void RegisterLuaUserType<SpriteAnimations>(sol::state& lua)
{
	if (!lua["SpriteAnimations"].valid())
	{
		lua.new_usertype<SpriteAnimations>("SpriteAnimations", "table", &SpriteAnimations::table);
	}
}