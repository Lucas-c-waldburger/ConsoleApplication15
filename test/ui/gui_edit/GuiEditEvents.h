#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../PropertyEditState.h"
#include "../../../events/data/EventDataIncludes.h"

namespace ui {

PropertyEditState GuiEditProperty(events::ContactCollisionBegin&);
PropertyEditState GuiEditProperty(events::ContactCollisionEnd&);
PropertyEditState GuiEditProperty(events::SensorCollisionBegin&);
PropertyEditState GuiEditProperty(events::SensorCollisionEnd&);
PropertyEditState GuiEditProperty(events::HitCollision&);

} // ui

#endif