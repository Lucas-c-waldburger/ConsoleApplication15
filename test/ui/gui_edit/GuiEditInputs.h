#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
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

bool GuiEditProperty(InputState& is);
bool GuiEditProperty(GameControllerInputSource& gcis);
bool GuiEditProperty(MouseInputSource& mis);
bool GuiEditProperty(GameControllerInputFieldValue& gcifv);
bool GuiEditProperty(SDL_MouseWheelDirection& mwd);
bool GuiEditProperty(MouseInputValues::CursorValue& mivcv);
bool GuiEditProperty(MouseInputValues::WheelValue& mivwv);
bool GuiEditProperty(MouseInputValues& miv);

bool GuiEditProperty(GameControllerInputMap& map);
bool GuiEditProperty(MouseInputMap& map);

bool GuiEditProperty(GameControllerInputField& gcif);
bool GuiEditProperty(MouseInputField& mif);

bool GuiEditProperty(GameControllerState& gcs);
bool GuiEditProperty(MouseState& ms);

} // ui

#endif