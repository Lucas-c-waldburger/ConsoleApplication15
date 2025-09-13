#pragma once
#include "EventStorage.h"

class EventBus2
{
public:
	// Connect to event types
	template <typename Fn>
	SignalToken ConnectToEvent(Fn&& fn) { return eventSignalList_.Connect(std::forward<Fn>(fn)); }

	template <SomeEventData T, typename Fn>
	SignalToken ConnectToEvent(Fn&& fn) { return eventSignalList_.Connect<T>(std::forward<Fn>(fn)); }

	// Connect specifically to events::GameControllerInput for a certain input source type
	template <typename Fn>
	SignalToken ConnectToInput(GameControllerInputSource src, Fn&& fn)
	{ 
		return controllerInputSignalList_.Connect(src, std::forward<Fn>(fn));
	}

	template <GameControllerInputSource src, typename Fn>
	SignalToken ConnectToInput(Fn&& fn)
	{
		return controllerInputSignalList_.Connect<src>(std::forward<Fn>(fn));
	}

	template <SomeEventData T>
	void PushEvent(T&& ev) { eventStorage_.Emplace(std::forward<T>(ev)); }
	 
	void DispatchEvents() { eventStorage_.Dispatch(eventSignalList_, controllerInputSignalList_); }

	void DiscardEvents() { eventStorage_.Discard(); }

	template <SomeEventData T>
	const auto& PeekEvents() const { return eventStorage_.Peek<T>(); }

	template <SomeEventData T>
	size_t NumEvents() const { return eventStorage_.NumEvents<T>(); }

private:
	EventSignalList eventSignalList_;
	ControllerInputSignalList controllerInputSignalList_;
	EventStorage eventStorage_;
};