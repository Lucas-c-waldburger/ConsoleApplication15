#pragma once
#include "../IEventData.h"
#include "../EventConcepts.h"

namespace events {

struct GameLoopStepStart : IEventData<GameLoopStepStart> {};

struct GameLoopStepEnd : IEventData<GameLoopStepEnd> {};

struct GameLoopStepRender : IEventData<GameLoopStepRender> {};

}