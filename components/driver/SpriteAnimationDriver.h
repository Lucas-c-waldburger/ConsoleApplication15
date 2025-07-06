#pragma once
#include "BaseDriver.h"
#include "../../sprite/SpriteAnimationSeries.h"

class SpriteAnimationDriver : public BaseDriver<SpriteAnimationDriver, SpriteAnimations>
{
public:
	friend class Super;

	// when switching series, how should the old one get reset? (NOTE: will not produce events!)
	enum ResetOption : uint8_t
	{
		None = 0,
		Index = 1 << 0,
		SpriteRange = 1 << 1,
		All = 0xFF
	};

	bool AddSeries(std::string_view seriesName, const SpriteSeriesAtlas& spriteAtlas);
	bool RemoveSeries(std::string_view seriesName);

	bool HasCurrentSeries() const;

	bool SetCurrentSeries(std::string_view seriesName, ResetOption resetOptions);
	bool SetCurrentSeries(std::string_view seriesName); // resetOptions = ALL

	bool SetCurrentSeriesIndex(size_t newIndex);
	bool SetCurrentSeriesRange(const Range<size_t>& newRange);

	bool Step();

	template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
	bool EnableCurrentSeriesEvents();
	template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
	bool DisableCurrentSeriesEvents();

private:
	static bool SeriesValid(const SpriteAnimationSeries& series);

	explicit SpriteAnimationDriver(Entity& entity) : Super(entity) {}

	void MarkNeedsUpdate();
};

template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
inline bool SpriteAnimationDriver::EnableCurrentSeriesEvents()
{
	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(animations.current);
	if (it == animations.end())
	{
		return false;
	}

	auto& series = it->second;
	assert(SeriesValid(series));

	if constexpr (sizeof...(Ts) == 0)
	{
		series.eventProductionFlags.EnableAll();
	}
	else
	{
		series.eventProductionFlags.Enable<Ts...>();
	}

	return true;
}

template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
inline bool SpriteAnimationDriver::DisableCurrentSeriesEvents()
{
	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(animations.current);
	if (it == animations.end())
	{
		return false;
	}

	auto& series = it->second;
	assert(SeriesValid(series));

	if constexpr (sizeof...(Ts) == 0)
	{
		series.eventProductionFlags.DisableAll();
	}
	else
	{
		series.eventProductionFlags.Disable<Ts...>();
	}

	return true;
}