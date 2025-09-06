#include "Counter.h"
#include <SDL_timer.h>

void Counter::Update()
{
	uint64_t now = SDL_GetPerformanceCounter();
	
	if (last_ == 0.0f)
	{
		last_ = now;
	}

	delta_ = static_cast<double>(now - last_) /
		static_cast<double>(SDL_GetPerformanceFrequency());

	last_ = now;
}