#pragma once
#include "EventBus.h"

template <typename Derived>
class EventDispatchListener 
{
public:
	// implements Dispatch()

	void ConnectToEventBus()
	{
		if (!token_.IsConnected())
		{
			token_ = EventBus::ConnectDispatchListener(GetDispatchCallback());
		}
	}

	void DisconnectFromEventBus()
	{
		EventBus::DisconnectDispatchListener(token_);
	}

private:
	auto GetDispatchCallback()
	{
		return [this](EventSpan span) -> void {
			static_cast<Derived*>(this)->Dispatch(span);
		};
	}

	EventDispatchListenerToken token_;
};