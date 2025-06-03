#include "EventCallbackMap.h"

bool EventCallbackMap::Erase(const Handle<EventCallback>& handle)
{
	if (!handle.IsValid())
	{
		return false;
	}

	auto it = map_.find(handle.eventType_);
	if (it == map_.end())
	{
		return false;
	}

	return EraseIf(it->second, [&handle](const auto& cb) {
		return handle == cb.handle;
		});
}
