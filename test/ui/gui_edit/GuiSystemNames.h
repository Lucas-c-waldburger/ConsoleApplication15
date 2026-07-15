#pragma once
#include "../../../FeatureFlags.h"
#include "../../../systems/SystemRegistry.h"
#include "../../../core/FixedString.h"
#include <string_view>

#if IMGUI_ENABLED

namespace ui {

template <FixedString Str, size_t I> 
    requires (I < static_cast<size_t>(std::numeric_limits<char>::max()))
consteval auto ConcatFixedStringWithSizeT()
{
    FixedString<Str.size() + 1> result{};

    for (size_t i = 0; i < Str.size() - 1; ++i)
    {
        result[i] = Str[i];
    }

    result[Str.size()] = static_cast<char>(I);

    return result;
}

template <typename T>
struct GuiSystemName;

template <typename T>
concept HasGuiSystemName = requires() {
	std::convertible_to<decltype(GuiSystemName<T>::name), std::string_view>;
	std::convertible_to<decltype(GuiSystemName<T>::label), std::string_view>;
};

#define DEF_GUI_SYS_NAME(sys) \
class sys; \
namespace ui { \
template <> struct GuiSystemName<sys> { \
 static constexpr std::string_view name = #sys; \
 static constexpr std::string_view label = "##"#sys; \
}; \
} // ui

} // ui

DEF_GUI_SYS_NAME(AudioSystem)
DEF_GUI_SYS_NAME(CameraSystem)
DEF_GUI_SYS_NAME(PhysicsSystem)
DEF_GUI_SYS_NAME(SpriteAnimationSystem)
DEF_GUI_SYS_NAME(TimerSystem)


#endif