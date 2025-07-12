#pragma once
#include "../deps/function2/function2.hpp"
#include "../core/commonObjects.h"

class Entity;

using StateTransitionCallback = fu2::unique_function<void(Entity&)>;
struct StateTransitionView
{
	HashName name;
	fu2::function_view<void(Entity&)> fn;
};
