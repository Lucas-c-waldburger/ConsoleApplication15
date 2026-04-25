#pragma once
#include "BaseComponent.h"
#include "../inputs/mouse/MouseCursor.h"
#include "../inputs/mouse/MouseInputMap.h"
#include <array>

struct MouseState : BaseComponent<MouseState>
{
    MouseInputMap inputs = MakeInputMap<MouseInputMap>();
    MouseInputValues values;
    //MouseCursorInputValue cursorValue;
    //MouseWheelInputValue wheelValue;
};

