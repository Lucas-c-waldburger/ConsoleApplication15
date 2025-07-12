#pragma once
#include "../LuaTypesRegistry.h"
#include "../../components/SpriteAnimationsComponent.h"


//template <> inline 
//void RegisterLuaUserType<GroupEventProductionFlags<events::SpriteAnimationEventGroup>>(sol::state& lua)
//{
//	if (!lua["GroupEventProductionFlags<events::SpriteAnimationEventGroup>"].valid())
//	{
//		lua.new_usertype<GroupEventProductionFlags<events::SpriteAnimationEventGroup>>(
//			"GroupEventProductionFlags<events::SpriteAnimationEventGroup>",
//			sol::constructors<GroupEventProductionFlags<events::SpriteAnimationEventGroup>()>(),
//
//			"EnableAll", &GroupEventProductionFlags<events::SpriteAnimationEventGroup>::EnableAll,
//			"DisableAll", & GroupEventProductionFlags<events::SpriteAnimationEventGroup>::DisableAll,
//
//			"EnableIndexChangeEvent", [](GroupEventProductionFlags<events::SpriteAnimationEventGroup>& self) { 
//				self.Enable<events::SpriteIndexChange>(); },
//			"DisableIndexChangeEvent", [](GroupEventProductionFlags<events::SpriteAnimationEventGroup>& self) {
//				self.Disable<events::SpriteIndexChange>(); },
//
//			"EnableSeriesChangeEvent", [](GroupEventProductionFlags<events::SpriteAnimationEventGroup>& self) {
//				self.Enable<events::SpriteSeriesChange>(); },
//			"DisableSeriesChangeEvent", [](GroupEventProductionFlags<events::SpriteAnimationEventGroup>& self) {
//				self.Disable<events::SpriteSeriesChange>(); },
//
//			"ShouldProduceIndexChangeEvent", [](const GroupEventProductionFlags<events::SpriteAnimationEventGroup>& self) {
//				return self.ShouldProduceEvent<events::SpriteIndexChange>(); },
//			"ShouldProduceSeriesChangeEvent", [](const GroupEventProductionFlags<events::SpriteAnimationEventGroup>& self) {
//				return self.ShouldProduceEvent<events::SpriteSeriesChange>(); }
//		);
//	}
//}


template <> inline void RegisterLuaUserType<SpriteAnimationSeries>(sol::state& lua)
{
	if (!lua["SpriteAnimationSeries"].valid())
	{
		lua.new_usertype<SpriteAnimationSeries>("SpriteAnimationSeries",
			"sourceAtlas", &SpriteAnimationSeries::sourceAtlas,
			"spritePlots", &SpriteAnimationSeries::spritePlots,
			"index", &SpriteAnimationSeries::index,
			"spriteRange", &SpriteAnimationSeries::spriteRange,
			"eventProductionFlags", &SpriteAnimationSeries::eventProductionFlags);
	}
}

//template <> inline void RegisterLuaUserType<SpriteAnimationsTable>(sol::state& lua)
//{
//	if (!lua["SpriteAnimationsTable"].valid())
//	{
//		lua.new_usertype<SpriteAnimationsTable>("SpriteAnimationsTable",
//			"NextInSeries", &SpriteAnimationsTable::NextInSeries,
//			"SetCurrentIndex", &SpriteAnimationsTable::SetCurrentIndex);
//	}
//}

template <> inline void RegisterLuaUserType<SpriteAnimations>(sol::state& lua)
{
	if (!lua["SpriteAnimations"].valid())
	{
		lua.new_usertype<SpriteAnimations>("SpriteAnimations",
			"get", [](SpriteAnimations& self, std::string_view name) 
			{
				return self.table[HashName{name}];
			},
			"set", [](SpriteAnimations& self, std::string_view name, const SpriteAnimationSeries& series)
			{
				self.table[HashName{name}] = series;
			},
			"remove", [](SpriteAnimations& self, std::string_view name)
			{
				self.table.erase(HashName{ name });
			},
			"current", &SpriteAnimations::current
		);
	}

}