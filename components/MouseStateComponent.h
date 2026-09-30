#pragma once
#include "ComponentConcepts.h"
#include "../inputs/mouse/MouseCursor.h"
#include "../inputs/mouse/MouseInputMap.h"
#include <array>

struct MouseState
{
    MouseInputMap inputs = MakeInputMap<MouseInputMap>();
    MouseInputValues values;
};

