#pragma once
#include "../callbacks/GameControllerCallbacks.h"
#include "../../events/handler/GameControllerEventHandler.h"
#include "DataEditDisplayUtils.h"


class ControllerMappingEditorContext
{
public:
    static inline const GameControllerEventHandler* controllerEventHandler = nullptr;
    static inline SignalToken axisImpulseSignalToken{};
};

class ControllerMappingEditor
{
public:
    static constexpr const char* kInputStateNames[] = {
        "None", "Pressed", "Released", "Held"
    };
    static constexpr const char* kControllerInputSourceNames[] = {
		"A", "B", "X", "Y", "Back", "Guide", "Start", "LeftStickButton",
		"RightStickButton", "LeftShoulder", "RightShoulder", "DPadUp", 
		"DPadDown", "DPadLeft", "DPadRight", "Misc1", "Paddle1", "Paddle2", 
        "Paddle3", "Paddle4", "Touchpad", "LeftStickAxis", "RightStickAxis", 
        "LeftTrigger", "RightTrigger"
    };

    static Result<Void> Init(const GameControllerEventHandler& handler);
    static bool IsInitialized();

    static bool DrawBodyLimits(BodyLimits& bodyLimits);

    static bool ConnectEntityToFirstController(Entity& e);

    static void DrawControllerMappingEditor(Entity& e, EventBus2& bus);

    static void DrawControllerState(Entity& e);

private:
    static bool IsEntityConnectedToController(const Entity& e);

    ControllerMappingEditor() = default;
};

