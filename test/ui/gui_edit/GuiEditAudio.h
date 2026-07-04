#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../../components/AudioComponents.h"

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

bool GuiEditProperty(AudioSpatialData& asd);
bool GuiEditProperty(AudioFadeMs& af);

bool GuiEditProperty(AudioChannelSettings& acs);
bool GuiEditProperty(AudioUpdateSettings& aus);

bool GuiEditProperty(AudioPlayCommand& apc);
bool GuiEditProperty(AudioStatus& as);

bool GuiEditProperty(NewAudioRequest& nar);
bool GuiEditProperty(AudioUpdateRequest& aur);
bool GuiEditProperty(ActiveAudio& aa);

} // ui

#endif