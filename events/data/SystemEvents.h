#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"
#include <SDL_events.h>

namespace events {

struct RenderReset : IEventData<RenderReset> {};

} // events