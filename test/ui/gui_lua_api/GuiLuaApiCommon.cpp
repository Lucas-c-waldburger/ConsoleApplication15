#include "GuiLuaApiCommon.h"

#if IMGUI_ENABLED
#include "../GuiTexture.h"

namespace ui {

namespace {

void GuiLuaRegisterCheckboxState(sol::state_view state)
{
	state.new_usertype<CheckboxState>("CheckboxState",
		"clicked", &CheckboxState::clicked,
		"current", &CheckboxState::current);
}

void GuiLuaRegisterSelectableState(sol::state_view state)
{
	state.new_usertype<SelectableState>("SelectableState",
		"clicked", &SelectableState::clicked,
		"current", &SelectableState::current);
}

void GuiLuaRegisterVecTypes(sol::state_view state)
{
	state.new_usertype<ImVec2>("Vec2",
		sol::constructors<ImVec2(), ImVec2(float, float)>(),
		"x", &ImVec2::x, "y", &ImVec2::y);

	state.new_usertype<ImVec4>("Vec4",
		sol::constructors<ImVec4(), ImVec4(float, float, float, float)>(),
		"x", &ImVec4::x, "y", &ImVec4::y, "z", &ImVec4::z, "w", &ImVec4::w);
}

void GuiLuaRegisterDragStateTypes(sol::state_view state)
{
	state.new_usertype<DragIntState>("DragIntState", "dragged", &DragIntState::dragged,
													 "value", &DragIntState::value);
	state.new_usertype<DragFloatState>("DragFloatState", "dragged", &DragFloatState::dragged,
									   "value", &DragFloatState::value);

	state.new_usertype<DragInt2State>("DragInt2State", "dragged", &DragInt2State::dragged,
		"value1", &DragInt2State::value1, "value2", &DragInt2State::value2);
	state.new_usertype<DragFloat2State>("DragFloat2State", "dragged", &DragFloat2State::dragged,
		"value1", &DragFloat2State::value1, "value2", &DragFloat2State::value2);

	state.new_usertype<DragInt3State>("DragInt3State", "dragged", &DragInt3State::dragged,
		"value1", &DragInt3State::value1, "value2", &DragInt3State::value2,
		"value3", &DragInt3State::value3);
	state.new_usertype<DragFloat3State>("DragFloat3State", "dragged", &DragFloat3State::dragged,
		"value1", &DragFloat3State::value1, "value2", &DragFloat3State::value2,
		"value3", &DragFloat3State::value3);

	state.new_usertype<DragInt4State>("DragInt4State", "dragged", &DragInt4State::dragged,
		"value1", &DragInt4State::value1, "value2", &DragInt4State::value2,
		"value3", &DragInt4State::value3, "value4", &DragInt4State::value4);
	state.new_usertype<DragFloat4State>("DragFloat4State", "dragged", &DragFloat4State::dragged,
		"value1", &DragFloat4State::value1, "value2", &DragFloat4State::value2,
		"value3", &DragFloat4State::value3, "value4", &DragFloat4State::value4);
}

void GuiLuaRegisterInputTextState(sol::state_view state)
{
	state.new_usertype<InputTextState>("InputTextState", 
		"input", &InputTextState::input,
		"value", &InputTextState::value);
}

void GuiLuaRegisterGuiTexture(sol::state_view state)
{
	state.new_usertype<GuiTexture>("GuiTexture", 
		"textureId", &GuiTexture::textureId,
		"size", &GuiTexture::size, 
		"uv0", &GuiTexture::uv0, 
		"uv1", &GuiTexture::uv1);
}

void GuiLuaRegisterStyles(sol::state_view state)
{
	state.new_usertype<ImGuiStyle>("GuiStyle", 
		"colors", &ImGuiStyle::Colors,
		"frame_rounding", &ImGuiStyle::FrameRounding, 
		"window_rounding", &ImGuiStyle::WindowRounding,
		"scrollbar_rounding", &ImGuiStyle::ScrollbarRounding, 
		"grab_rounding", &ImGuiStyle::GrabRounding);
}

} // unnamed

void GuiLuaRegisterCommonTypes(sol::state_view state)
{
	GuiLuaRegisterVecTypes(state);
	GuiLuaRegisterDragStateTypes(state);
	GuiLuaRegisterCheckboxState(state);
	GuiLuaRegisterSelectableState(state);
	GuiLuaRegisterInputTextState(state);
	GuiLuaRegisterGuiTexture(state);
}

} // ui

#endif