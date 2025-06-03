#pragma once
#include <array>
#include <SDL.h>
#include <SDL_events.h>
#include <array>
#include <unordered_set>

template <size_t N> requires (N > 0)
class EventBuffer
{
public:
	EventBuffer()
	{
		events_.fill(SDL_Event{ .type = SDL_POLLSENTINEL });
	}

	bool Push(SDL_Event&& ev) 
	{ 
		if (Full())
		{
			LOG_WARNING("Event buffer full");
			return false;
		}

		assert(nextFree_ < N);

		events_[nextFree_] = std::move(ev); 

		nextFree_ = (nextFree_ + 1) % N;

		return true;
	}

	SDL_Event Pop()
	{
		if (Empty())
		{
			return SDL_Event{ .type = SDL_POLLSENTINEL };
		}

		assert(headIndex_ < N);

		SDL_Event ev = std::move(events_[headIndex_]);

		headIndex_ = (headIndex_ + 1) % N;

		return ev;
	}

	std::unordered_set<uint32_t> PeekEventTypes(std::optional<EventCode> ofCode = {}) const
	{
		assert(headIndex_ < N);
		
		std::unordered_set<uint32_t> eventTypes;

		auto process = [&](const SDL_Event& ev) -> void {
			if (ev.type == SDL_POLLSENTINEL)
			{
				return;
			}

			if (ofCode.has_value() && GetEventCode(ev) != *ofCode)
			{
				return;
			}

			eventTypes.insert(ev.type);
		};

		size_t start = headIndex_;
		size_t end = nextFree_;

		if (end < start) // wrapped around
		{
			for (size_t i = 0; i < end; i++)
			{
				process(events_[i]);
			}

			end = N;
		}

		for (size_t i = start; i < end; i++)
		{
			process(events_[i]);
		}

		return eventTypes;
	}

	bool Empty() const
	{
		return headIndex_ == nextFree_;
	}

	bool Full() const
	{
		return (nextFree_ + 1) % N == headIndex_;
	}

	size_t Size() const
	{
		size_t size = 0;

		if (nextFree_ < headIndex_) // wrapped around
		{
			size = N - headIndex_ + nextFree_;
		}
		else
		{
			size = nextFree_ - headIndex_;
		}

		return size;
	}

	constexpr size_t Capacity() const noexcept { return N - 1; }

	void Reset()
	{
		headIndex_ = 0;
		nextFree_ = 0;
	}

private:
	std::array<SDL_Event, N> events_;
	size_t headIndex_ = 0;
	size_t nextFree_ = 0;
};