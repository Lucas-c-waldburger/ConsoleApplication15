#pragma once
#include "../../../FeatureFlags.h"
#include <string_view>

#if IMGUI_ENABLED

namespace ui {

template <typename T>
struct GuiEventName;

template <typename T>
concept HasGuiEventName = requires() {
	std::convertible_to<decltype(GuiEventName<T>::name), std::string_view>;
	std::convertible_to<decltype(GuiEventName<T>::label), std::string_view>;
};

#define DEF_GUI_EVENT_NAME(ev) \
namespace events { struct ev; } \
namespace ui { \
template <> struct GuiEventName<events::ev> { \
 static constexpr std::string_view name = #ev; \
 static constexpr std::string_view label = "##"#ev; \
}; \
} // ui

} // ui

DEF_GUI_EVENT_NAME(ContactCollisionBegin)
DEF_GUI_EVENT_NAME(ContactCollisionEnd)
DEF_GUI_EVENT_NAME(SensorCollisionBegin)
DEF_GUI_EVENT_NAME(SensorCollisionEnd)
DEF_GUI_EVENT_NAME(HitCollision)
DEF_GUI_EVENT_NAME(GameControllerConnected)
DEF_GUI_EVENT_NAME(GameControllerDisconnected)
DEF_GUI_EVENT_NAME(GameControllerInput)
DEF_GUI_EVENT_NAME(MouseInput)
DEF_GUI_EVENT_NAME(KeyboardInput)

#endif