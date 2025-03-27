#include "EventBuffer.h"
#include <cassert>
#include <SDL.h>

void EventBuffer::Push(SDL_Event ev)
{
	if (size_ >= kMaxSize) // full up
	{
		assert(size_ == kMaxSize);
		assert(headIndex_ > -1);

		// we will insert this event overtop current event at 
		// head index and increment the head index to treat the next as current
		if (headIndex_ >= kMaxSize - 1)
		{
			assert(headIndex_ == kMaxSize - 1);

			// incrementing head index would be out of range, wrap around
			events_[kMaxSize - 1] = std::move(ev);
			headIndex_ = 0;
		}
		else // head somewhere between 0 & 62
		{
			events_[headIndex_] = std::move(ev);
			++headIndex_;
		}
	}
	else // not full
	{
		// move ev to 'back of array'
		events_[(headIndex_ + size_) % kMaxSize] = std::move(ev);
		++size_;
	}
}

SDL_Event EventBuffer::Pop()
{
	if (size_ <= 0)
	{
		assert(size_ == 0);

		// we may be somewhere else in the array, set to 0 so
		// next event drain is consistent
		headIndex_ = 0;

		SDL_Event sentinel{ .type = SDL_POLLSENTINEL };

		return sentinel;
	}

	// we know size > 0, grab event and increment
	assert(headIndex_ > -1);

	SDL_Event retEvent = events_[headIndex_];

	--size_;

	headIndex_ = (headIndex_ + 1) % 64;

	return retEvent;
}