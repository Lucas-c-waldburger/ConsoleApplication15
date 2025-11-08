#pragma once
#include "../core/Signal.h"
#include "EventSignalConcepts.h"
#include "data/EventDataIncludes.h"

template <SomeEventData T>
using EventSignal = Signal<const T&>;

namespace detail {
template <typename TList>
struct event_signal_list;

template <SomeEventData...Ts>
struct event_signal_list<TypeList<Ts...>> {
	using type = std::tuple<EventSignal<Ts>...>;
};
} // detail

using event_signal_list_t = typename detail::event_signal_list<EventDataTypeList>::type;

class EventSignalList
{
public:
	EventSignalList() = default;
	~EventSignalList() = default;

	EventSignalList(const EventSignalList&) = delete;
	EventSignalList& operator=(const EventSignalList&) = delete;

	EventSignalList(EventSignalList&& other) noexcept : signals_(std::move(other.signals_)) {}
	EventSignalList& operator=(EventSignalList&& other) noexcept
	{
		if (this != &other)
		{
			signals_ = std::move(other.signals_);
		}
		return *this;
	}

	template <SomeEventData T, ValidEventSignalFnOfType<T> Fn>
	SignalToken Connect(Fn&& fn)
	{
		return std::get<EventSignal<T>>(signals_).Connect(std::forward<Fn>(fn));
	}

	// will auto-deduce event data from Fn signature without specifying T
	template <ValidEventSignalFn Fn> 
	SignalToken Connect(Fn&& fn)
	{
		using event_data_t = valid_signal_fn<Fn>::raw_event_data_type;

		return std::get<EventSignal<event_data_t>>(signals_).Connect(std::forward<Fn>(fn));
	}

	template <SomeEventData T>
	void Emit(const T& ev)
	{
		return std::get<EventSignal<T>>(signals_).Emit(ev);
	}

private:
	event_signal_list_t signals_;
};
