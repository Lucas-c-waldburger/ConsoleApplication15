#pragma once
#include <SDL_timer.h>

class Counter
{
public:
	float GetDelta() const
	{
		return static_cast<float>(delta_);
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