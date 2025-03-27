#pragma once
#include <array>

union SDL_Event;

class EventBuffer
{
public:
	static constexpr int kMaxSize = 64;

	void Push(SDL_Event ev);
	SDL_Event Pop();
	bool Empty() const { return size_ <= 0; }
	size_t Size() const { return size_; }

private:
	std::array<SDL_Event, kMaxSize> events_;
	int headIndex_ = 0;
	int size_ = 0;
};

