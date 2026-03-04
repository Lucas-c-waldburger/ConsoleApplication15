#pragma once
#include "EventStorage.h"

class EventBus2
{
public:
	// Connect to event types
	template <ValidEventSignalFn Fn>
	SignalToken ConnectToEvent(Fn&& fn) 
	{ 
		return signalLists_.eventSignals.Connect(std::forward<Fn>(fn)); 
	}

	template <SomeEventData T, ValidEventSignalFnOfType<T> Fn>
	SignalToken ConnectToEvent(Fn&& fn) 
	{ 
		return signalLists_.eventSignals.Connect<T>(std::forward<Fn>(fn));
	}

	template <ValidUserEventSignalFn Fn> 
		requires (!SomeEventData<std::remove_cvref_t<
			typename func_traits<Fn>::template arg_at<0>>>)
	SignalToken ConnectToEvent(Fn&& fn)
	{
		return signalLists_.userEventSignals.Connect(std::forward<Fn>(fn));
	}

	// connect to input events for a particular input source
	template <SomeInputSourceEnum Source, ValidInputSignalFn<Source> Fn>
	SignalToken ConnectToInput(Source src, Fn&& fn)
	{
		return signalLists_.GetInputSignalList<Source>().Connect(src, std::forward<Fn>(fn));
	}

	template <SomeInputEvent T, SomeInputSourceEnum Source, 
		      ValidInputSignalFnOfType<Source, T> Fn>
	SignalToken ConnectToInput(Source src, Fn&& fn)
	{
		return signalLists_.GetInputSignalList<Source>().Connect(src, std::forward<Fn>(fn));
	}

	template <typename T>
	void PushEvent(T&& ev) 
	{ 
		eventStorage_.Emplace(std::forward<T>(ev));
	}

	template <SomeEventData T>
	void PushEvents(std::vector<T>&& evs) { eventStorage_.EmplaceRange(std::move(evs)); }

	template <typename T>
	void PushAndDispatchEvents(T&& ev)
	{
		PushEvent(std::forward<T>(ev));
		DispatchEvents();
	}
	 
	void DispatchEvents() { eventStorage_.Dispatch(signalLists_); }

	void DiscardEvents() { eventStorage_.Discard(); }

	template <SomeEventData T>
	const auto& PeekEvents() const { return eventStorage_.Peek<T>(); }

	template <SomeEventData T>
	size_t NumEvents() const { return eventStorage_.NumEvents<T>(); }

	EventStorage& GetEventStorage() { return eventStorage_; }

private:
	SignalListCollection signalLists_;
	EventStorage eventStorage_;
};