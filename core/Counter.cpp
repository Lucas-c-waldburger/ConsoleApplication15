#include "Counter.h"
#include <SDL_timer.h>
#include <limits>
#include <algorithm>

float Counter::GetDelta() const
{
    return static_cast<float>(std::min(
        static_cast<double>(std::numeric_limits<float>::max()),
        delta_));
}

void Counter::Update()
{
    uint64_t now = SDL_GetPerformanceCounter();

    if (last_ == 0) 
    {
        last_ = now;
        delta_ = 0.0;

        return;
    }

    if (last_ > now)
    {
        int x = 0;
    }
    delta_ = static_cast<double>(now - last_) / static_cast<double>(SDL_GetPerformanceFrequency());

    last_ = now;
}