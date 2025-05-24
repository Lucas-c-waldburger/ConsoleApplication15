#pragma once
#include "BaseComponent.h"
#include <optional>
#include <SDL_rect.h>
#include "../deps/function2/function2.hpp"
#include "../core/Monitoring.h"

template <typename T>
struct MetricCache
{
	T total;
	T lastSnapshot;
};

template <typename T>
struct ProgressTrigger
{
	T threshold;
	fu2::unique_function<ReturnSignal()> onThresholdCrossed;
};

template <typename T>
struct EntityMetric
{
	T total;
	std::vector<ProgressTrigger<T>> triggers;
};

namespace detail{
template <typename T> struct is_entity_metric : std::false_type {};
template <typename T> struct is_entity_metric<EntityMetric<T>> : std::true_type {};
} 

template <typename T>
inline constexpr bool is_entity_metric_v = detail::is_entity_metric<T>::value;


struct EntityMetrics : public BaseComponent<EntityMetrics, 13>
{
	EntityMetric<double> activeTime;
	std::optional<EntityMetric<SDL_FPoint>> travelDistance;
};

//struct Sprites : public BaseComponent<Sprites, 13>
//{
//	Handle<SpriteSeriesAtlas> sourceAtlas;
//	std::string seriesName;
//	size_t currentIndex = 0;
//};
