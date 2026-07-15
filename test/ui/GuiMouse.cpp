#include "GuiMouse.h"

#if IMGUI_ENABLED
#include "InspectorCommon.h"

namespace ui {

using Src = MouseInputSource;
using State = InputState;

bool GuiMouse::CheckState(MouseInputSource src, InputState st)
{
	assert(entity_.IsValid());
	return entity_.GetComponent<MouseState>().inputs[src].state == st;
}

void GuiMouse::Init() 
{
	if (entity_.IsValid())
	{
		assert(entity_.HasComponent<InspectorTag>());
		assert(entity_.HasComponent<MouseState>());

		return;
	}

	entity_ = ECS::CreateEntity();
	assert(entity_.IsValid());

	ECS::RegisterComponent<InspectorTag>();

	entity_.AddComponent<InspectorTag>();
	entity_.AddComponent<MouseState>();
}

SDL_FPoint GuiMouse::GetPosition()
{
	assert(entity_.IsValid());
	return entity_.GetComponent<MouseState>().values.cursor.absolutePos;
}

SDL_FPoint GuiMouse::GetRelativePosition()
{
	assert(entity_.IsValid());
	return entity_.GetComponent<MouseState>().values.cursor.relativePos;
}

bool GuiMouse::IsLeftClicked()
{
	return CheckState(Src::LeftButton, State::Pressed);
}

bool GuiMouse::IsLeftHeld()
{
	return CheckState(Src::LeftButton, State::Held);
}

bool GuiMouse::IsLeftReleased()
{
	return CheckState(Src::LeftButton, State::Released);
}

bool GuiMouse::IsRightClicked()
{
	return CheckState(Src::RightButton, State::Pressed);
}

bool GuiMouse::IsRightHeld()
{
	return CheckState(Src::RightButton, State::Held);
}

bool GuiMouse::IsRightReleased()
{
	return CheckState(Src::RightButton, State::Released);
}

bool GuiMouse::IsMiddleClicked()
{
	return CheckState(Src::MiddleButton, State::Pressed);
}

bool GuiMouse::IsMiddleHeld()
{
	return CheckState(Src::MiddleButton, State::Held);
}

bool GuiMouse::IsMiddleReleased()
{
	return CheckState(Src::MiddleButton, State::Released);
}

bool GuiMouse::IsWheelScrolled()
{
	return CheckState(Src::Wheel, State::Pressed);
}

bool GuiMouse::InsideEditorWindow()
{
	return ImGui::GetIO().WantCaptureMouse;
}

float GuiMouse::GetScrollY()
{
	assert(entity_.IsValid());
	return entity_.GetComponent<MouseState>().values.wheel.scroll.y;
}

} // ui

#endif