#include "EventDispatchListener.h"
#include "EventBus.h"

void EventDispatchListenerToken::Disconnect()
{
	if (IsConnected())
	{
		EventBus::DisconnectDispatchListener(*this);

		id_ = kInvalidId;
	}
}
