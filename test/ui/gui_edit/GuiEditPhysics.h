#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../../../components/RigidBodyComponent.h"
#include "../../../components/ColliderComponent.h"

namespace ui {

bool GuiEdit(Force& f, const char* label = "");
bool GuiEdit(ForceRequests& fr, const char* label = "");
bool GuiEdit(BodyLimits& bl, const char* label = "");

bool GuiEditProperty(Force& f);
bool GuiEditProperty(ForceRequests& fr);
bool GuiEditProperty(BodyLimits& bl, ImGuiTreeNodeFlags flags = 0);
bool GuiEditProperty(B2Body::Type& bt);
bool GuiEditProperty(B2Shape::Type& st);

bool GuiEditProperty(RigidBody& rb);
bool GuiEditProperty(Collider& col);

} // ui

#endif
