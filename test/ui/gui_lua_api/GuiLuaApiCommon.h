#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <sol/sol.hpp>

namespace ui {

struct CheckboxState
{
	bool clicked = false;
	bool current = false;
};

template <typename T>
struct DragState
{
	bool dragged = false;
	T value;
};
using DragIntState = DragState<int>;
using DragFloatState = DragState<float>;

template <typename T>
struct Drag2State
{
	bool dragged = false;
	T value1;
	T value2;
};
using DragInt2State = Drag2State<int>;
using DragFloat2State = Drag2State<float>;

template <typename T>
struct Drag3State
{
	bool dragged = false;
	T value1;
	T value2;
	T value3;
};
using DragInt3State = Drag3State<int>;
using DragFloat3State = Drag3State<float>;

template <typename T>
struct Drag4State
{
	bool dragged = false;
	T value1;
	T value2;
	T value3;
	T value4;
};
using DragInt4State = Drag4State<int>;
using DragFloat4State = Drag4State<float>;

struct InputTextState
{
	bool input = false;
	std::string value;
};

struct MenuItemState
{
	bool clicked = false;
	bool enabled = false;
};

struct SelectableState
{
	bool clicked = false;
	bool current = false;
};

void GuiLuaRegisterCommonTypes(sol::state_view state);


} // ui

#endif