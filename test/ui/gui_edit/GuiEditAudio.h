#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../../components/AudioComponents.h"
#include "../PropertyEditState.h"

namespace ui {

bool GuiEdit(AudioSpatialData& asd, const char* label = "");
bool GuiEdit(AudioFadeMs& af, const char* label = "");

bool GuiEdit(AudioChannelSettings& acs, const char* label = "");
bool GuiEdit(AudioUpdateSettings& aus, const char* label = "");

bool GuiEdit(AudioPlayCommand& apc, const char* label = "");
bool GuiEdit(AudioStatus& as, const char* label = "");

bool GuiEdit(NewAudioRequest& nar, const char* label = "");
bool GuiEdit(AudioUpdateRequest& aur, const char* label = "");
bool GuiEdit(ActiveAudio& aa, const char* label = "");

PropertyEditState GuiEditProperty(AudioSpatialData& asd);
PropertyEditState GuiEditProperty(AudioFadeMs& af);

PropertyEditState GuiEditProperty(AudioChannelSettings& acs);
PropertyEditState GuiEditProperty(AudioUpdateSettings& aus);

PropertyEditState GuiEditProperty(AudioPlayCommand& apc);
PropertyEditState GuiEditProperty(AudioStatus& as);

PropertyEditState GuiEditProperty(NewAudioRequest& nar);
PropertyEditState GuiEditProperty(AudioUpdateRequest& aur);
PropertyEditState GuiEditProperty(ActiveAudio& aa);

} // ui

#endif