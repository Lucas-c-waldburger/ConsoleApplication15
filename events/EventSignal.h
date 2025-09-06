#pragma once
#include "../core/Signal.h"
#include "data/EventDataIncludes.h"
#include "../core/FuncTraits.h"
#include "../inputs/controller/GameControllerInputSource.h"

template <SomeEventData T>
using EventSignal = Signal<const T&>;

template <typename Fn>
concept EventSignalCompliantFn = requires() {
	HasFuncTraits<Fn>; // is a function
	std::same_as<typename func_traits<Fn>::return_type, void>; // returns void
	func_traits<Fn>::arg_types::size == 1; // single argument
	SomeEventData<type_at_index_t<0, typename func_traits<Fn>::arg_types>>; // arg is an event data type
	is_const_reference_v<type_at_index_t<0, typename func_traits<Fn>::arg_types>>; // arg is const T&
};

namespace detail {
	template <typename TList>
	struct event_signal_list;

	template <SomeEventData...Ts>
	struct event_signal_list<TypeList<Ts...>> {
		using type = std::tuple<EventSignal<Ts>...>;
	};
} // detail

using event_signal_list_t = typename detail::event_signal_list<EventDataTypeList>::type;

template <typename Fn, SomeEventData T>
static constexpr bool convertible_to_slot_callback_v =
	std::convertible_to<Fn, typename EventSignal<T>::SlotCallbackType>;

template <EventSignalCompliantFn Fn>
using extracted_raw_event_data_t =
	raw_type_t<type_at_index_t<0, typename func_traits<Fn>::arg_types>>;

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

	template <SomeEventData T, typename Fn> requires convertible_to_slot_callback_v<Fn, T>
	SignalToken Connect(Fn&& fn)
	{
		return std::get<EventSignal<T>>(signals_).Connect(std::forward<Fn>(fn));
	}

	// will auto-deduce event data from Fn signature without specifying T
	template <typename Fn> 
		requires convertible_to_slot_callback_v<Fn, extracted_raw_event_data_t<Fn>>
	SignalToken Connect(Fn&& fn)
	{
		using EventDataType = extracted_raw_event_data_t<Fn>;

		return std::get<EventSignal<EventDataType>>(signals_).Connect(std::forward<Fn>(fn));
	}

	template <SomeEventData T>
	void Emit(const T& ev)
	{
		return std::get<EventSignal<T>>(signals_).Emit(ev);
	}

private:
	event_signal_list_t signals_;
};
