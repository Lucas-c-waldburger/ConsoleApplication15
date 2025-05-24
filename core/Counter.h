#pragma once
#include <SDL_timer.h>

class Counter
{
public:
	double GetDelta() const
	{
		return delta_;
	}

	void Update()
	{
		uint64_t now = SDL_GetPerformanceCounter();

		delta_ = static_cast<double>(now - last_) /
				 static_cast<double>(SDL_GetPerformanceFrequency());

		last_ = now;
	}

private:
	uint64_t last_ = 0;
	double delta_ = 0.0;
};