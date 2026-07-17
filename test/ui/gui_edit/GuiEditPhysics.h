#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../PropertyEditState.h"
#include "../../../components/RigidBodyComponent.h"
#include "../../../components/ColliderComponent.h"

namespace ui {

bool GuiEdit(Force& f, const char* label = "");
bool GuiEdit(ForceRequests& fr, const char* label = "");
bool GuiEdit(BodyLimits& bl, const char* label = "");

PropertyEditState GuiEditProperty(Force& f);
PropertyEditState GuiEditProperty(ForceRequests& fr);
PropertyEditState GuiEditProperty(BodyLimits& bl, ImGuiTreeNodeFlags flags = 0);
PropertyEditState GuiEditProperty(B2Body::Type& bt);
PropertyEditState GuiEditProperty(B2Shape::Type& st);

PropertyEditState GuiEditProperty(RigidBody& rb);
PropertyEditState GuiEditProperty(Collider& col);

} // ui

#endif
