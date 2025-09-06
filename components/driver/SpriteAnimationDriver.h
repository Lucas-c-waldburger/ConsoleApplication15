#pragma once
#include "BaseDriver.h"
#include "../../components/SpriteAnimationsComponent.h"

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
	
	std::string_view GetCurrentSeriesName() const;
	size_t GetCurrentSeriesSize() const;
	size_t GetCurrentSeriesIndex() const;
	Range<size_t> GetCurrentSeriesRange() const;

	bool SetCurrentSeries(std::string_view seriesName, ResetOption resetOptions);
	bool SetCurrentSeries(std::string_view seriesName); // resetOptions = ALL

	bool SetCurrentSeriesIndex(size_t newIndex);
	bool SetCurrentSeriesRange(const Range<size_t>& newRange);

	bool AdvanceCurrentSeries();

	template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
	bool EnableCurrentSeriesEvents();
	template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
	bool DisableCurrentSeriesEvents();

private:
	SpriteAnimationSeries* GetCurrentSeries();
	const SpriteAnimationSeries* GetCurrentSeries() const;

	static bool SeriesValid(const SpriteAnimationSeries& series);

	explicit SpriteAnimationDriver(Entity entity) : Super(entity) {}

	void MarkNeedsUpdate();
};

template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
inline bool SpriteAnimationDriver::EnableCurrentSeriesEvents()
{
	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(animations.current);
	if (it == animations.table.end())
	{
		return false;
	}

	auto& series = it->second;
	assert(SeriesValid(series));

	if constexpr (sizeof...(Ts) == 0)
	{
		series.eventProductionFlags.set();
	}
	else
	{
		((series.eventProductionFlags.set(Ts::eventType)), ...);
	}

	return true;
}

template <typename...Ts> requires (SomeTypeInList<Ts, events::SpriteAnimationEventGroup> && ...)
inline bool SpriteAnimationDriver::DisableCurrentSeriesEvents()
{
	auto& animations = GetComponent<SpriteAnimations>();

	auto it = animations.table.find(animations.current);
	if (it == animations.table.end())
	{
		return false;
	}

	auto& series = it->second;
	assert(SeriesValid(series));

	if constexpr (sizeof...(Ts) == 0)
	{
		series.eventProductionFlags.reset();
	}
	else
	{
		((series.eventProductionFlags.set(Ts::eventType, false)), ...);
	}

	return true;
}