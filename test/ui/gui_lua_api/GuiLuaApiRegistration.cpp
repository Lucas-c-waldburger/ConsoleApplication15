#include "GuiLuaApiRegistration.h"

#if IMGUI_ENABLED
#include <magic_enum/magic_enum.hpp>
#include <boost/pfr.hpp>
#include <concepts>
#include <string_view>
#include <array>
#include <format>
#include <ranges>
#include <sstream>
#include <iomanip>
#include <misc/cpp/imgui_stdlib.h>
#include "../GuiTexture.h"

namespace ui {

namespace {

static constexpr std::array kFmtArgStrings{
	"{}",
	"{}{}",
	"{}{}{}",
	"{}{}{}{}",
	"{}{}{}{}{}",
	"{}{}{}{}{}{}",
	"{}{}{}{}{}{}{}",
	"{}{}{}{}{}{}{}{}"
};

std::string_view StripImGuiPrefixFromEnumValue(std::string_view sv)
{
	if (auto pos = sv.find('_'); pos != std::string_view::npos) 
	{
		sv.remove_prefix(pos + 1);
	}

	return sv;
}

template <typename E, size_t...Is>
inline void RegisterEnumImpl(sol::state_view state, std::string_view enumName,
	const std::array<std::pair<E, std::string_view>, sizeof...(Is)>& enumEntries,
	std::index_sequence<Is...>)
{
	auto args = std::tuple_cat(
		std::forward_as_tuple(
		StripImGuiPrefixFromEnumValue(enumEntries[Is].second), enumEntries[Is].first)...
	);

	std::apply([&](auto&&...args) {
		state.new_enum(enumName, std::forward<decltype(args)>(args)...);
	}, args);
}

template <typename E> requires std::is_enum_v<E>
inline void RegisterEnum(sol::state_view state, std::string_view enumName)
{
	static constexpr auto enumEntries = magic_enum::enum_entries<E>();

	RegisterEnumImpl(state, enumName, enumEntries, std::make_index_sequence<enumEntries.size()>{});
}

void GuiLuaRegisterEnums(sol::state_view state)
{
	RegisterEnum<ImGuiWindowFlags_>(state, "WindowFlags");
	RegisterEnum<ImGuiCol_>(state, "StyleColor");
	RegisterEnum<ImGuiStyleVar_>(state, "StyleVar");
	RegisterEnum<ImGuiComboFlags_>(state, "ComboFlags");
	RegisterEnum<ImGuiPopupFlags_>(state, "PopupFlags");
	RegisterEnum<ImGuiTabBarFlags_>(state, "TabBarFlags");
	RegisterEnum<ImGuiTabItemFlags_>(state, "TabItemFlags");
	RegisterEnum<ImGuiHoveredFlags_>(state, "HoveredFlags");
	RegisterEnum<ImGuiSelectableFlags_>(state, "SelectableFlags");
	RegisterEnum<ImGuiMouseButton_>(state, "MouseButton");
	RegisterEnum<ImGuiKey>(state, "KeyboardKey");
}

void GuiLuaPushStyleColor(ImGuiCol col, ImVec4 vec4)
{
	ImGui::PushStyleColor(col, vec4);
}
void GuiLuaPushStyleVar(ImGuiStyleVar var, ImVec4 vec4)
{
	ImGui::PushStyleColor(var, vec4);
}
std::string GuiLuaGetTextHelper(sol::variadic_args args)
{
	if (args.size() == 1 && args.begin()->is<std::string>())
	{
		return args.begin()->as<std::string>();
	}

	std::stringstream ss;

	for (const auto& arg : args)
	{
		if (arg.is<std::string>())
		{
			ss << arg.get<std::string>();
		}
		else if (arg.is<int>())
		{
			ss << arg.get<int>();
		}
		else if (arg.is<double>())
		{
			ss << arg.get<double>();
		}
		else if (arg.is<bool>())
		{
			ss << std::boolalpha << arg.get<bool>() << std::noboolalpha;
		}
	}

	return ss.str();
}

void GuiLuaTextUnformatted(sol::variadic_args args)
{
	const auto str = GuiLuaGetTextHelper(args);

	ImGui::TextUnformatted(str.c_str());
}

void GuiLuaTextColored(ImVec4 color, sol::variadic_args args)
{
	const auto str = GuiLuaGetTextHelper(args);

	ImGui::TextColored(color, str.c_str());
}

CheckboxState GuiLuaCheckbox(const char* label, bool curState)
{
	const bool clicked = ImGui::Checkbox(label, &curState);

	return { .clicked = clicked, .current = curState };
}

template <typename T>
DragState<T> GuiLuaDrag(const char* label, T curVal)
{
	bool dragged = false;
	if constexpr (std::same_as<T, int>)
	{
		dragged = ImGui::DragInt(label, &curVal);
	}
	else
	{
		dragged = ImGui::DragFloat(label, &curVal);
	}

	return { .dragged = dragged, .value = curVal };
}

template <typename T>
Drag2State<T> GuiLuaDrag2(const char* label, T curVal1, T curVal2)
{
	bool dragged = false;
	T arr[2] = { curVal1, curVal2 };

	if constexpr (std::same_as<T, int>)
	{
		dragged = ImGui::DragInt2(label, arr);
	}
	else
	{
		dragged = ImGui::DragFloat2(label, arr);
	}

	return { .dragged = dragged, 
		     .value1 = arr[0], .value2 = arr[1] };
}

template <typename T>
Drag3State<T> GuiLuaDrag3(const char* label, T curVal1, T curVal2, T curVal3)
{
	bool dragged = false;
	T arr[3] = { curVal1, curVal2, curVal3 };

	if constexpr (std::same_as<T, int>)
	{
		dragged = ImGui::DragInt3(label, arr);
	}
	else
	{
		dragged = ImGui::DragFloat3(label, arr);
	}

	return { .dragged = dragged, 
		     .value1 = arr[0], .value2 = arr[1], .value3 = arr[2] };
}

template <typename T>
Drag4State<T> GuiLuaDrag4(const char* label, T curVal1, T curVal2, T curVal3, T curVal4)
{
	bool dragged = false;
	T arr[4] = { curVal1, curVal2, curVal3, curVal4 };

	if constexpr (std::same_as<T, int>)
	{
		dragged = ImGui::DragInt4(label, arr);
	}
	else
	{
		dragged = ImGui::DragFloat4(label, arr);
	}

	return { .dragged = dragged,
			 .value1 = arr[0], .value2 = arr[1], .value3 = arr[2], .value4 = arr[3] };
}

InputTextState GuiLuaInputText(const char* label, std::string curText)
{
	const bool input = ImGui::InputText(label, &curText);

	return { .input = input, .value = std::move(curText) };
}

bool GuiLuaMenuItem(const char* label)
{
	return ImGui::MenuItem(label);
}

SelectableState GuiLuaSelectable(const char* label, bool curState)
{
	const bool clicked = ImGui::Selectable(label, &curState);

	return { .clicked = clicked, .current = curState };
}

SelectableState GuiLuaSelectableFlags(const char* label, bool curState, int flags)
{
	const bool clicked = ImGui::Selectable(label, &curState, flags);

	return { .clicked = clicked, .current = curState };
}

} // unnamed

static constexpr std::string_view kGuiLuaBeginWindowName = "begin_window";
static constexpr std::string_view kGuiLuaEndWindowName = "end_window";
static constexpr std::string_view kGuiLuaBeginChildName = "begin_child";
static constexpr std::string_view kGuiLuaEndChildName = "end_child";
static constexpr std::string_view kGuiLuaSelectableName = "selectable";
static constexpr std::string_view kGuiLuaGetContentRegionMaxName = "get_content_region_max";
static constexpr std::string_view kGuiLuaGetContentRegionAvailName = "get_content_region_avail";
static constexpr std::string_view kGuiLuaGetScrollXName = "get_scroll_x";
static constexpr std::string_view kGuiLuaGetScrollYName = "get_scroll_y";
static constexpr std::string_view kGuiLuaGetScrollHereXName = "set_scroll_here_x";
static constexpr std::string_view kGuiLuaGetScrollHereYName = "set_scroll_here_y";
static constexpr std::string_view kGuiLuaPushStyleColName = "push_style_color";
static constexpr std::string_view kGuiLuaPopStyleColName = "pop_style_color";
static constexpr std::string_view kGuiLuaPushStyleVarName = "push_style_var";
static constexpr std::string_view kGuiLuaPopStyleVarName = "pop_style_var";
static constexpr std::string_view kGuiLuaSeparatorName = "separator";
static constexpr std::string_view kGuiLuaSameLineName = "same_line";
static constexpr std::string_view kGuiLuaDummyName = "dummy";
static constexpr std::string_view kGuiLuaIndentName = "indent";
static constexpr std::string_view kGuiLuaUnindentName = "unindent";
static constexpr std::string_view kGuiLuaGetCursorPosName = "get_cursor_pos";
static constexpr std::string_view kGuiLuaGetCursorPosXName = "get_cursor_pos_x";
static constexpr std::string_view kGuiLuaGetCursorPosYName = "get_cursor_pos_y";
static constexpr std::string_view kGuiLuaSetCursorPosName = "set_cursor_pos";
static constexpr std::string_view kGuiLuaSetCursorPosXName = "set_cursor_pos_x";
static constexpr std::string_view kGuiLuaSetCursorPosYName = "set_cursor_pos_y";
static constexpr std::string_view kGuiLuaGetFrameHeightName = "get_frame_height";
static constexpr std::string_view kGuiLuaPushIDName = "push_id";
static constexpr std::string_view kGuiLuaPopIDName = "pop_id";
static constexpr std::string_view kGuiLuaBeginDisabledName = "begin_disabled";
static constexpr std::string_view kGuiLuaEndDisabledName = "end_disabled";
static constexpr std::string_view kGuiLuaTextUnformattedName = "text_unformatted";
static constexpr std::string_view kGuiLuaTextColoredName = "text_colored";
static constexpr std::string_view kGuiLuaButtonName = "button";
static constexpr std::string_view kGuiLuaCheckboxName = "checkbox";
static constexpr std::string_view kGuiLuaBeginComboName = "begin_combo";
static constexpr std::string_view kGuiLuaEndComboName = "end_combo";
static constexpr std::string_view kGuiLuaDragIntName = "drag_int";
static constexpr std::string_view kGuiLuaDragFloatName = "drag_float";
static constexpr std::string_view kGuiLuaDragInt2Name = "drag_int2";
static constexpr std::string_view kGuiLuaDragFloat2Name = "drag_float2";
static constexpr std::string_view kGuiLuaDragInt3Name = "drag_int3";
static constexpr std::string_view kGuiLuaDragFloat3Name = "drag_float3";
static constexpr std::string_view kGuiLuaDragInt4Name = "drag_int4";
static constexpr std::string_view kGuiLuaDragFloat4Name = "drag_float4";
static constexpr std::string_view kGuiLuaInputTextName = "input_text";
static constexpr std::string_view kGuiLuaBeginMainMenuBarName = "begin_main_menu_bar";
static constexpr std::string_view kGuiLuaEndMainMenuBarName = "end_main_menu_bar";
static constexpr std::string_view kGuiLuaBeginMenuBarName = "begin_menu_bar";
static constexpr std::string_view kGuiLuaEndMenuBarName = "end_menu_bar";
static constexpr std::string_view kGuiLuaMenuItemName = "menu_item";
static constexpr std::string_view kGuiLuaBeginTooltipName = "begin_tooltip";
static constexpr std::string_view kGuiLuaEndTooltipName = "end_tooltip";
static constexpr std::string_view kGuiLuaOpenPopupName = "open_popup";
static constexpr std::string_view kGuiLuaBeginPopupName = "begin_popup";
static constexpr std::string_view kGuiLuaEndPopupName = "end_popup";
static constexpr std::string_view kGuiLuaBeginTabBarName = "begin_tab_bar";
static constexpr std::string_view kGuiLuaEndTabBarName = "end_tab_bar";
static constexpr std::string_view kGuiLuaBeginTabItemName = "begin_tab_item";
static constexpr std::string_view kGuiLuaEndTabItemName = "end_tab_item";
static constexpr std::string_view kGuiLuaIsItemHoveredName = "is_item_hovered";
static constexpr std::string_view kGuiLuaIsItemActiveName = "is_item_active";
static constexpr std::string_view kGuiLuaIsItemFocusedName = "is_item_focused";
static constexpr std::string_view kGuiLuaIsItemClickedName = "is_item_clicked";
static constexpr std::string_view kGuiLuaCalcTextSizeName = "calc_text_size";
static constexpr std::string_view kGuiLuaIsMouseDownName = "is_mouse_down";
static constexpr std::string_view kGuiLuaIsMouseClickedName = "is_mouse_clicked";
static constexpr std::string_view kGuiLuaIsMouseReleasedName = "is_mouse_released";
static constexpr std::string_view kGuiLuaIsMouseDoubleClickedName = "is_mouse_double_clicked";
static constexpr std::string_view kGuiLuaGetMousePosName = "get_mouse_pos";
static constexpr std::string_view kGuiLuaIsKeyDownName = "is_key_down";
static constexpr std::string_view kGuiLuaIsKeyPressedName = "is_key_pressed";
static constexpr std::string_view kGuiLuaIsKeyReleasedName = "is_key_released";
static constexpr std::string_view kGuiLuaGetUiTextureName = "get_ui_texture";
static constexpr std::string_view kGuiLuaGetGameTextureName = "get_game_texture";
static constexpr std::string_view kGuiLuaImageName = "image";
static constexpr std::string_view kGuiLuaImageButtonName = "image_button";

void GuiLuaRegisterStandaloneFunctions(sol::table& tbl)
{
	tbl[kGuiLuaBeginWindowName] = sol::overload(
		[](const char* nm) { ImGui::Begin(nm); },
		[](const char* nm, int flg) { ImGui::Begin(nm, nullptr, flg); });
	tbl[kGuiLuaEndWindowName] = &ImGui::End;
	tbl[kGuiLuaBeginChildName] = sol::overload(
		[](const char* nm) { ImGui::BeginChild(nm); },
		[](const char* nm, int flg) { ImGui::BeginChild(nm, ImVec2(0,0), false, flg); },
		[](const char* nm, ImVec2 sz) { ImGui::BeginChild(nm, sz); },
		[](const char* nm, ImVec2 sz, bool brdr, int flg) { ImGui::BeginChild(nm, sz, brdr, flg); });
	tbl[kGuiLuaSelectableName] = sol::overload(&GuiLuaSelectable, &GuiLuaSelectableFlags);
	tbl[kGuiLuaEndChildName] = &ImGui::EndChild;
	tbl[kGuiLuaGetContentRegionMaxName] = &ImGui::GetContentRegionMax;
	tbl[kGuiLuaGetContentRegionAvailName] = &ImGui::GetContentRegionAvail;
	tbl[kGuiLuaGetScrollXName] = &ImGui::GetScrollX;
	tbl[kGuiLuaGetScrollYName] = &ImGui::GetScrollY;
	tbl[kGuiLuaGetScrollHereXName] = &ImGui::SetScrollHereX;
	tbl[kGuiLuaGetScrollHereYName] = &ImGui::SetScrollHereY;
	tbl[kGuiLuaPushStyleColName] = &GuiLuaPushStyleColor;
	tbl[kGuiLuaPopStyleColName] = sol::overload(
		[]() { ImGui::PopStyleColor(); },
		[](int i) { ImGui::PopStyleColor(i); });
	tbl[kGuiLuaPushStyleVarName] = &GuiLuaPushStyleVar;
	tbl[kGuiLuaPopStyleVarName] = sol::overload(
		[]() { ImGui::PopStyleVar(); },
		[](int i) { ImGui::PopStyleVar(i); });
	tbl[kGuiLuaSeparatorName] = &ImGui::Separator;
	tbl[kGuiLuaSameLineName] = sol::overload(
		[]() { ImGui::SameLine(); },									   
		[](float off) { ImGui::SameLine(off); },										  
		[](float off, float sp) { ImGui::SameLine(off, sp); });
	tbl[kGuiLuaDummyName] = &ImGui::Dummy;
	tbl[kGuiLuaIndentName] = sol::overload(
		[]() { ImGui::Indent(); },
		[](float w) { ImGui::Indent(w); });
	tbl[kGuiLuaUnindentName] = sol::overload(
		[]() { ImGui::Unindent(); },
		[](float w) { ImGui::Unindent(w); });
	tbl[kGuiLuaGetCursorPosName] = &ImGui::GetCursorPos;
	tbl[kGuiLuaGetCursorPosXName] = &ImGui::GetCursorPosX;
	tbl[kGuiLuaGetCursorPosYName] = &ImGui::GetCursorPosY;
	tbl[kGuiLuaSetCursorPosName] = &ImGui::SetCursorPos;
	tbl[kGuiLuaSetCursorPosXName] = &ImGui::SetCursorPosX;
	tbl[kGuiLuaSetCursorPosYName] = &ImGui::SetCursorPosY;
	tbl[kGuiLuaGetFrameHeightName] = &ImGui::GetFrameHeight;
	tbl[kGuiLuaPushIDName] = sol::overload(
		[](const std::string& id) { ImGui::PushID(id.c_str()); },
		[](int id) { ImGui::PushID(id); });
	tbl[kGuiLuaPopIDName] = &ImGui::PopID;
	tbl[kGuiLuaBeginDisabledName] = sol::overload(
		[]() { ImGui::BeginDisabled(); },
		[](bool b) { ImGui::BeginDisabled(b); });
	tbl[kGuiLuaEndDisabledName] = &ImGui::EndDisabled;
	tbl[kGuiLuaTextUnformattedName] = &GuiLuaTextUnformatted;
	tbl[kGuiLuaTextColoredName] = &GuiLuaTextColored;
	tbl[kGuiLuaButtonName] = sol::overload(
		[](const char* str) { return ImGui::Button(str); },
		[](const char* str, ImVec2 size) { return ImGui::Button(str, size); });
	tbl[kGuiLuaCheckboxName] = &GuiLuaCheckbox;
	tbl[kGuiLuaBeginComboName] = sol::overload(
		[](const char* lbl, const char* prv) { return ImGui::BeginCombo(lbl, prv); },
		[](const char* lbl, const char* prv, int flg) { return ImGui::BeginCombo(lbl, prv, flg); });
	tbl[kGuiLuaEndComboName] = &ImGui::EndCombo;
	tbl[kGuiLuaDragIntName] = &GuiLuaDrag<int>;
	tbl[kGuiLuaDragFloatName] = &GuiLuaDrag<float>;
	tbl[kGuiLuaDragInt2Name] = &GuiLuaDrag2<int>;
	tbl[kGuiLuaDragFloat2Name] = &GuiLuaDrag2<float>;
	tbl[kGuiLuaDragInt3Name] = &GuiLuaDrag3<int>;
	tbl[kGuiLuaDragFloat3Name] = &GuiLuaDrag3<float>;
	tbl[kGuiLuaDragInt4Name] = &GuiLuaDrag4<int>;
	tbl[kGuiLuaDragFloat4Name] = &GuiLuaDrag4<float>;
	tbl[kGuiLuaInputTextName] = &GuiLuaInputText;
	tbl[kGuiLuaBeginMenuBarName] = &ImGui::BeginMenuBar;
	tbl[kGuiLuaEndMenuBarName] = &ImGui::EndMenuBar;
	tbl[kGuiLuaBeginMainMenuBarName] = &ImGui::BeginMainMenuBar;
	tbl[kGuiLuaEndMainMenuBarName] = &ImGui::EndMainMenuBar;
	tbl[kGuiLuaMenuItemName] = &GuiLuaMenuItem;
	tbl[kGuiLuaBeginTooltipName] = &ImGui::BeginTooltip;
	tbl[kGuiLuaEndTooltipName] = &ImGui::EndTooltip;
	tbl[kGuiLuaOpenPopupName] = sol::overload(
		[](const char* nm) { ImGui::OpenPopup(nm); },
		[](const char* nm, int flg) { ImGui::OpenPopup(nm, flg); });
	tbl[kGuiLuaBeginPopupName] = sol::overload(
		[](const char* nm) { return ImGui::BeginPopup(nm); },
		[](const char* nm, int flg) { return ImGui::BeginPopup(nm, flg); });
	tbl[kGuiLuaEndPopupName] = &ImGui::EndPopup;
	tbl[kGuiLuaBeginTabBarName] = sol::overload(
		[](const char* nm) { return ImGui::BeginTabBar(nm); },
		[](const char* nm, int flg) { return ImGui::BeginTabBar(nm, flg); });
	tbl[kGuiLuaEndTabBarName] = &ImGui::EndTabBar;
	tbl[kGuiLuaBeginTabItemName] = sol::overload(
		[](const char* nm) { return ImGui::BeginTabItem(nm); },
		[](const char* nm, int flg) { return ImGui::BeginTabItem(nm, nullptr, flg); });
	tbl[kGuiLuaEndTabBarName] = &ImGui::EndTabItem;
	tbl[kGuiLuaIsItemHoveredName] = sol::overload(
		[]() { return ImGui::IsItemHovered(); },
		[](int flg) { return ImGui::IsItemHovered(flg); });
	tbl[kGuiLuaIsItemActiveName] = &ImGui::IsItemActive;
	tbl[kGuiLuaIsItemFocusedName] = &ImGui::IsItemFocused;
	tbl[kGuiLuaIsItemClickedName] = sol::overload(
		[]() { return ImGui::IsItemClicked(); },
		[](int btn) { return ImGui::IsItemClicked(btn); });
	tbl[kGuiLuaCalcTextSizeName] = [](const char* txt) { return ImGui::CalcTextSize(txt); };
	tbl[kGuiLuaIsMouseDownName] = &ImGui::IsMouseDown;
	tbl[kGuiLuaIsMouseClickedName] = sol::overload(
		[](int btn) { return ImGui::IsMouseClicked(btn); },
		[](int btn, bool rep) { return ImGui::IsMouseClicked(btn, rep); });
	tbl[kGuiLuaIsMouseReleasedName] = &ImGui::IsMouseReleased;
	tbl[kGuiLuaIsMouseDoubleClickedName] = &ImGui::IsMouseDoubleClicked;
	tbl[kGuiLuaGetMousePosName] = &ImGui::GetMousePos;
	tbl[kGuiLuaIsKeyDownName] = &ImGui::IsKeyDown;
	tbl[kGuiLuaIsKeyPressedName] = sol::overload(
		[](ImGuiKey key) { return ImGui::IsKeyPressed(key); },
		[](ImGuiKey key, bool rep) { return ImGui::IsKeyPressed(key, rep); });
	tbl[kGuiLuaIsKeyReleasedName] = &ImGui::IsKeyReleased;
}

void GuiLuaRegisterTextureFunctions(sol::table& tbl, const GuiTextureSources& txSrcs)
{
	tbl[kGuiLuaGetUiTextureName] = [&txSrcs](const std::string& spName) {
		return GuiTextureConverter{ txSrcs.uiTextures }.FromSprite(spName);
	};

	tbl[kGuiLuaGetGameTextureName] = [&txSrcs](const std::string& spName) {
		return GuiTextureConverter{ txSrcs.gameTextures }.FromSprite(spName);
	};

	tbl[kGuiLuaImageName] = &GuiImage;

	tbl[kGuiLuaImageButtonName] = sol::overload(
		[](const std::string& id, const GuiTexture& tx) { return GuiImageButton(id, tx); },
		[](const std::string& id, const GuiTexture& tx, ImVec4 bgClr) {
			return GuiImageButton(id, tx, bgClr); },
		[](const std::string& id, const GuiTexture& tx, ImVec4 bgClr, ImVec4 tintClr) {
			return GuiImageButton(id, tx, bgClr, tintClr); });

}

void GuiLuaRegister(sol::state_view state, const GuiTextureSources& txSrcs)
{
	GuiLuaRegisterEnums(state);
	GuiLuaRegisterCommonTypes(state);

	sol::table tbl = state.create_named_table("gui");

	GuiLuaRegisterStandaloneFunctions(tbl);
	GuiLuaRegisterTextureFunctions(tbl, txSrcs);
}

} // ui

#endif