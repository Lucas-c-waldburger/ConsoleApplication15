#pragma once
#include "BaseComponent.h"
#include "../inputs/InputData.h"
#include "../inputs/mouse/MouseCursor.h"
#include "../core/ShiftedIndexArray.h"
#include "../inputs/mouse/MouseInputMap.h"
#include <array>

struct MouseState : BaseComponent<MouseState>
{
    MouseInputMap inputs = MakeMouseInputMap();
    MouseCursorInputValue cursorValue;
    MouseWheelInputValue wheelValue;
};

