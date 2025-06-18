#pragma once
#include <array>
#include <span>
#include <vector>
#include "Event.h"
#include "EventConcepts.h"

using EventSpan = std::span<const Event>;

template <size_t N>
class EventStage
{
public:
	template <SomeEventData T>
	void StageEvents(const std::vector<T>& eventDatas)
	{
		if (effectiveSize_ + eventDatas.size() > N)
		{
			std::cout << "EventStage full";
			return;
		}

		for (const auto& evData : eventDatas)
		{
			auto& ev = buffer_[effectiveSize_++];
			ev.type = T::eventType;
			ev.timestamp = evData.timestamp;
			ev.data = static_cast<const void*>(&evData);
		}
	}

	EventSpan GetStagedEvents()
	{
		assert(effectiveSize_ <= N);

		return { buffer_.data(), effectiveSize_ };
	}

	void ClearStage() { effectiveSize_ = 0; }

	size_t Size() const { return effectiveSize_; }

private:
	std::array<Event, N> buffer_;
	size_t effectiveSize_ = 0;
};