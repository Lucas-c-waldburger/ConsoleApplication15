#pragma once
#include <array>
#include <SDL.h>
#include <queue>

//class EventBuffer
//{
//public:
//	static constexpr int kMaxSize = 64;
//
//	void Push(SDL_Event ev);
//	SDL_Event Pop();
//	bool Empty() const { return size_ <= 0; }
//	size_t Size() const { return size_; }
//
//private:
//	std::array<SDL_Event, kMaxSize> events_{};
//	int headIndex_ = 0;
//	int size_ = 0;
//};

class EventBuffer
{
public:
	void Push(SDL_Event ev) { events_.push(std::move(ev)); }
	SDL_Event Pop() 
	{ 
		if (events_.empty())
		{
			return SDL_Event{ .type = SDL_POLLSENTINEL };
		}

		SDL_Event ev = std::move(events_.front());
		events_.pop();

		return ev;
	}
	bool Empty() const { return events_.empty(); }
	size_t Size() const { return events_.size(); }

private:
	std::queue<SDL_Event> events_{};
};

