#include "GuiEditEvents.h"

#if IMGUI_ENABLED

namespace ui {

PropertyEditState GuiEditProperty(events::ContactCollisionBegin&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::ContactCollisionEnd&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::SensorCollisionBegin&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::SensorCollisionEnd&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::HitCollision&) { return PropertyEditState::None; }



} // ui

#endif