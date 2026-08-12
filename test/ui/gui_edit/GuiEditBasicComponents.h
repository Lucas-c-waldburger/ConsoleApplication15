#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../PropertyEditState.h"
#include "../../../components/TransformComponent.h"
#include "../../../components/CameraTargetComponent.h"
#include "../../../components/RelationComponents.h"
#include "../../../components/TagsComponent.h"
#include "../../../components/TimerComponent.h"
#include "../../../components/NameComponent.h"
#include "../../../components/SignalTokenStorageComponent.h"

namespace ui {

bool GuiEdit(Transform& tf);
bool GuiEdit(CameraTarget& ct);
bool GuiEdit(Parent& p);
bool GuiEdit(Children& ch);
bool GuiEdit(Tags& tg);
bool GuiEdit(Timer& tmr);
bool GuiEdit(Name& nm);

PropertyEditState GuiEditProperty(Transform& tf);
PropertyEditState GuiEditProperty(CameraTarget& ct);
PropertyEditState GuiEditProperty(Parent& p);
PropertyEditState GuiEditProperty(Children& ch);
PropertyEditState GuiEditProperty(Tags& tg);
PropertyEditState GuiEditProperty(Timer& tmr);
PropertyEditState GuiEditProperty(Name& nm);
PropertyEditState GuiEditProperty(SignalTokenStorage&);

} // ui

#endif