#pragma once
#include "../deps/function2/function2.hpp"
#include "../core/Monitoring.h"
#include "EventHandle.h"

struct EventCallback
{
    fu2::unique_function<ReturnSignal(const SDL_Event& ev)> onEvent;
    Handle<EventCallback> handle;
};

//struct EventCallback
//{
//    fu2::unique_function<ReturnSignal(const Event& ev)> onEvent;
//    Handle<EventCallback> handle;
//};
