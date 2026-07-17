#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../PropertyEditState.h"
#include "../../../components/GameControllerStateComponent.h"
#include "../../../components/MouseStateComponent.h"

namespace ui {

bool GuiEdit(InputState& is, const char* label = "");
bool GuiEdit(GameControllerInputSource& gcis, const char* label = "");
bool GuiEdit(MouseInputSource& mis, const char* label = "");
bool GuiEdit(GameControllerInputFieldValue& gcifv, const char* label = "");
bool GuiEdit(SDL_MouseWheelDirection& mwd, const char* label = "");
bool GuiEdit(MouseInputValues::CursorValue& mivcv, const char* label = "");
bool GuiEdit(MouseInputValues::WheelValue& mivwv, const char* label = "");
bool GuiEdit(MouseInputValues& miv, const char* label = "");

bool GuiEdit(GameControllerInputMap& map, const char* label = "");
bool GuiEdit(MouseInputMap& map, const char* label = "");

bool GuiEdit(GameControllerInputField& gcif, const char* label = "");
bool GuiEdit(MouseInputField& mif, const char* label = "");

bool GuiEdit(GameControllerState& gcs);
bool GuiEdit(MouseState& ms);

PropertyEditState GuiEditProperty(InputState& is);
PropertyEditState GuiEditProperty(GameControllerInputSource& gcis);
PropertyEditState GuiEditProperty(MouseInputSource& mis);
PropertyEditState GuiEditProperty(GameControllerInputFieldValue& gcifv);
PropertyEditState GuiEditProperty(SDL_MouseWheelDirection& mwd);
PropertyEditState GuiEditProperty(MouseInputValues::CursorValue& mivcv);
PropertyEditState GuiEditProperty(MouseInputValues::WheelValue& mivwv);
PropertyEditState GuiEditProperty(MouseInputValues& miv);

PropertyEditState GuiEditProperty(GameControllerInputMap& map);
PropertyEditState GuiEditProperty(MouseInputMap& map);

PropertyEditState GuiEditProperty(GameControllerInputField& gcif);
PropertyEditState GuiEditProperty(MouseInputField& mif);

PropertyEditState GuiEditProperty(GameControllerState& gcs);
PropertyEditState GuiEditProperty(MouseState& ms);

} // ui

#endif