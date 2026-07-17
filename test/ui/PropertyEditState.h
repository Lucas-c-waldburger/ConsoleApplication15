#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <cstdint>

namespace ui {

enum PropertyEditState : uint8_t
{
    None = 0,
    Started = 1 << 0,
    Active = 1 << 1,
    Finished = 1 << 2
};

inline constexpr PropertyEditState& operator|=(PropertyEditState& lhs, PropertyEditState rhs) 
{
    lhs = static_cast<PropertyEditState>(lhs | rhs);
    return lhs;
}

inline PropertyEditState EvaluatePropertyState()
{
    uint8_t state = PropertyEditState::None;

    if (ImGui::IsItemActivated())
    {
        state |= PropertyEditState::Started;
    }
    if (ImGui::IsItemActive())
    {
        state |= PropertyEditState::Active;
    }
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        state |= PropertyEditState::Finished;
    }

    return static_cast<PropertyEditState>(state);
}

} // ui


#endif