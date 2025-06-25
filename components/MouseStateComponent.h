#pragma once
#include "BaseComponent.h"
#include "../inputs/InputData.h"
#include "../inputs/mouse/MouseCursor.h"
#include "../core/ShiftedIndexArray.h"
#include <array>

//struct MouseState : BaseComponent<MouseState, 1>
struct MouseState : BaseComponent<MouseState>
{
    // Handle<MouseCursor> activeCursor;
    MousePositionInputData positionData;
    ShiftedIndexArray<MouseButtonInputData, 5, -1> buttonData;
};