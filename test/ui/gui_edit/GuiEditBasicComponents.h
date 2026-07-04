#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../../components/TransformComponent.h"
#include "../../../components/CameraTargetComponent.h"
#include "../../../components/RelationComponents.h"
#include "../../../components/TagsComponent.h"
#include "../../../components/TimerComponent.h"
#include "../../../components/NameComponent.h"

namespace ui {

bool GuiEdit(Transform& tf);
bool GuiEdit(CameraTarget& ct);
bool GuiEdit(Parent& p);
bool GuiEdit(Children& ch);
bool GuiEdit(Tags& tg);
bool GuiEdit(Timer& tmr);
bool GuiEdit(Name& nm);

bool GuiEditProperty(Transform& tf);
bool GuiEditProperty(CameraTarget& ct);
bool GuiEditProperty(Parent& p);
bool GuiEditProperty(Children& ch);
bool GuiEditProperty(Tags& tg);
bool GuiEditProperty(Timer& tmr);
bool GuiEditProperty(Name& nm);

} // ui

#endif