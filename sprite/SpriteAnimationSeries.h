#pragma once
#include <algorithm>
#include "../atlas/SpriteSeriesAtlas.h"
#include "../core/Literals.h"
#include "../sdl/SDLUtils.h"
#include "../events/data/SpriteAnimationEvents.h"
#include "../core/EventProductionFlags.h"

struct SpriteAnimationSeries
{
	Handle<SpriteSeriesAtlas> sourceAtlas;
	std::vector<AtlasPlot> spritePlots;
	size_t index = 0;
	Range<size_t> spriteRange = { 0, 0 };
	EventGroupProductionFlags<events::SpriteAnimationEventGroup> eventProductionFlags;
};

//using SpriteAnimationPolicy = fu2::function_view<void(SpriteSeriesAnimationData&)>;
//
//inline void DefaultSpriteAdvancePolicy(SpriteSeriesAnimationData& animData)
//{
//	animData.index = (animData.index + 1) % animData.spritePlots.size();
//}
//inline void DefaultSeriesResetPolicy(SpriteSeriesAnimationData& animData)
//{
//	animData.index = 0;
//}
//
//inline void KnightJumpSpriteAdvancePolicy(SpriteSeriesAnimationData& animData)
//{
//	if (animData.index != animData.spritePlots.size() - 1)
//	{
//		DefaultSpriteAdvancePolicy(animData);
//	}
//}
//
//struct SpriteSeriesAnimation
//{
//	SpriteSeriesAnimationData data;
//	SpriteAnimationPolicy advance = &DefaultSpriteAdvancePolicy;
//};
//
//class SpriteSeriesAnimationTable
//{
//
//};
//
//struct SpriteAnimationComponent
//{
//	Handle<SpriteSeriesAnimationTable> tableHandle;
//
//};

//template <typename T>
//concept SomeAnimationPolicy = requires() {
//	std::invocable<typename T::Step, SpriteAnimationSeries&>;
//	std::invocable<typename T::Reset, SpriteAnimationSeries&>;
//};
//
//struct DefaultAnimationPolicy
//{
//	static void Step(SpriteAnimationSeries& series)
//	{
//
//	}
//};

//template <typename T>
//concept SomeAnimationStepPolicy = std::invocable

//class SpriteSeriesAtlas;
//
//struct SpriteAnimationSeries
//{
//    Handle<SpriteSeriesAtlas> sourceAtlas;
//    std::vector<AtlasPlot> spritePlots;
//    size_t index = 0;
//    Range<size_t> spriteRange = { 0, 0 };
//
//    friend bool operator==(const SpriteAnimationSeries& lhs, const SpriteAnimationSeries& rhs)
//    {
//        return lhs.sourceAtlas == rhs.sourceAtlas && lhs.spritePlots == rhs.spritePlots;
//    }
//};

//template <typename T, typename F>
//struct Dirtyable
//{
//    friend F;
//
//    Dirtyable& operator=(const T& val)
//    {
//        value_ = val;
//        dirty_ = true;
//        return *this;
//    }
//
//    operator const T& () const { return value_; }
//
//private:
//    T value_;
//    bool dirty_ = true;
//};
//
//class SpriteAnimationSystem;
//
//template <typename T>
//using SpriteAnimationDirtyable = Dirtyable<T, SpriteAnimationSystem>;
//
//struct SpriteAnimationSeries2
//{
//    SpriteAnimationDirtyable<Handle<SpriteSeriesAtlas>> sourceAtlas;
//    SpriteAnimationDirtyable<std::vector<AtlasPlot>> spritePlots;
//    SpriteAnimationDirtyable<size_t> index;
//    SpriteAnimationDirtyable<Range<size_t>> spriteRange;
//
//    friend bool operator==(const SpriteAnimationSeries& lhs, const SpriteAnimationSeries& rhs)
//    {
//        return lhs.sourceAtlas == rhs.sourceAtlas && lhs.spritePlots == rhs.spritePlots;
//    }
//};


//friend void operator++(SpriteAnimationSeries& series)
//{
//    series.index = (series.index + 1) % series.spritePlots.size();
//}
//
//friend void operator++(SpriteAnimationSeries& series, int)
//{
//    series.index = (series.index + 1) % series.spritePlots.size();
//}
//
//friend void operator--(SpriteAnimationSeries& series)
//{
//    size_t newIdx = std::max(0_uz, series.spritePlots.size());
//    series.index = (newIdx == 0) ? 0 : series.spritePlots.size() - 1;
//}
//
//friend void operator--(SpriteAnimationSeries& series, int)
//{
//    size_t newIdx = std::max(0_uz, series.spritePlots.size());
//    series.index = (newIdx == 0) ? 0 : series.spritePlots.size() - 1;
//}