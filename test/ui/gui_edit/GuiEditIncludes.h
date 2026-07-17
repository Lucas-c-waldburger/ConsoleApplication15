#pragma once

#include "GuiEditCore.h"
#include "GuiEditBasicComponents.h"
#include "GuiEditRenderable.h"
#include "GuiEditAudio.h"
#include "GuiEditIncludes.h"
#include "GuiEditPhysics.h"
#include "../PropertyEditState.h"

#if IMGUI_ENABLED

namespace ui {

template <typename T>
concept HasGuiEdit = requires(T& t) {
	{ GuiEdit(t) } -> std::convertible_to<bool>;
};

template <typename T>
concept HasGuiEditProperty = requires(T & t) {
	{ GuiEditProperty(t) } -> std::convertible_to<PropertyEditState>;
};


} // ui

#endif