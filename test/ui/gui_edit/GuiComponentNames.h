#pragma once
#include "../../../FeatureFlags.h"
#include <string_view>

#if IMGUI_ENABLED

namespace ui {

template <typename T>
struct GuiComponentName;

template <typename T>
concept HasGuiComponentName = requires() {
	std::convertible_to<decltype(GuiComponentName<T>::name), std::string_view>;
	std::convertible_to<decltype(GuiComponentName<T>::label), std::string_view>;
};

#define DEF_GUI_CMP_NAME(cmp) \
struct cmp; \
namespace ui { \
template <> struct GuiComponentName<cmp> { \
 static constexpr std::string_view name = #cmp; \
 static constexpr std::string_view label = "##"#cmp; \
}; \
} // ui

#define DEF_GUI_CMP_CUSTOM_NAME(cmp, customName) \
struct cmp; \
namespace ui { \
template <> struct GuiComponentName<cmp> { \
 static constexpr std::string_view name = customName; \
 static constexpr std::string_view label = "##"#customName; \
}; \
} // ui

} // ui

//DEF_GUI_CMP_NAME(ActiveState);
DEF_GUI_CMP_NAME(MouseState)
DEF_GUI_CMP_NAME(Transform)
DEF_GUI_CMP_NAME(RigidBody)
DEF_GUI_CMP_NAME(Parent)
DEF_GUI_CMP_NAME(Children)
DEF_GUI_CMP_NAME(Tags)
DEF_GUI_CMP_NAME(Collider)
DEF_GUI_CMP_NAME(GameControllerState)
DEF_GUI_CMP_NAME(CameraTarget)
DEF_GUI_CMP_NAME(Timer);
//DEF_GUI_CMP_NAME(EntityFlags);
DEF_GUI_CMP_NAME(NewAudioRequest)
DEF_GUI_CMP_NAME(AudioUpdateRequest)
DEF_GUI_CMP_NAME(ActiveAudio)
DEF_GUI_CMP_NAME(TextRenderableGlyphCache)
DEF_GUI_CMP_NAME(TextRenderableComponent)
DEF_GUI_CMP_NAME(SpriteRenderableComponent)
//DEF_GUI_CMP_NAME(MarkedDestroyed);
DEF_GUI_CMP_NAME(SpriteAnimationComponent)
//DEF_GUI_CMP_NAME(NeedsAnimationUpdate);
DEF_GUI_CMP_NAME(Name)

DEF_GUI_CMP_CUSTOM_NAME(SignalTokenStorage, "Callbacks")

#endif
