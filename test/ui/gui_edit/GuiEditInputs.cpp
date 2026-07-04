#include "GuiEditInputs.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "GuiEditPropertyTable.h"

namespace ui {

bool GuiEdit(InputState& is, const char* label)
{
	static constexpr const char* kNames[] = {
		"None",
		"Pressed",
		"Released",
		"Held"
	};
	int cur = is == InputState::None ? 0 :
			  is == InputState::Pressed ? 1 :
			  is == InputState::Released ? 2 : 3;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: is = InputState::None; break;
		case 1: is = InputState::Pressed; break;
		case 2: is = InputState::Released; break;
		case 3: is = InputState::Held; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(GameControllerInputSource& gcis, const char* label)
{
	using Src = GameControllerInputSource;

	static constexpr const char* kNames[] = {
		"Invalid",
		"A",
		"B",
		"X",
		"Y",
		"Back",
		"Guide",
		"Start",
		"LeftStickButton",
		"RightStickButton",
		"LeftShoulder",
		"RightShoulder",
		"DPadUp",
		"DPadDown",
		"DPadLeft",
		"DPadRight",
		"Misc1",
		"Paddle1",
		"Paddle2",
		"Paddle3",
		"Paddle4",
		"TouchPad",
		"LeftStickAxis",
		"RightStickAxis",
		"LeftTrigger",
		"RightTrigger"
	};
	int cur = gcis == Src::Invalid ? 0 :
		      gcis == Src::A ? 1 :
		      gcis == Src::B ? 2 :
		      gcis == Src::X ? 3 :
		      gcis == Src::Y ? 4 :
		      gcis == Src::Back ? 5 :
		      gcis == Src::Guide ? 6 :
		      gcis == Src::Start ? 7 :
		      gcis == Src::LeftStickButton ? 8 :
		      gcis == Src::RightStickButton ? 9 :
		      gcis == Src::LeftShoulder ? 10 :
		      gcis == Src::RightShoulder ? 11 :
		      gcis == Src::DPadUp ? 12 :
		      gcis == Src::DPadDown ? 13 :
		      gcis == Src::DPadLeft ? 14 :
		      gcis == Src::DPadRight ? 15 :
		      gcis == Src::Misc1 ? 16 :
		      gcis == Src::Paddle1 ? 17 :
		      gcis == Src::Paddle2 ? 18 :
		      gcis == Src::Paddle3 ? 19 :
		      gcis == Src::Paddle4 ? 20 :
		      gcis == Src::TouchPad ? 21 :
		      gcis == Src::LeftStickAxis ? 22 :
		      gcis == Src::RightStickAxis ? 23 :
		      gcis == Src::LeftTrigger ? 24 : 25;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: gcis = Src::Invalid; break;
		case 1: gcis = Src::A; break;
		case 2: gcis = Src::B; break;
		case 3: gcis = Src::X; break;
		case 4: gcis = Src::Y; break;
		case 5: gcis = Src::Back; break;
		case 6: gcis = Src::Guide; break;
		case 7: gcis = Src::Start; break;
		case 8: gcis = Src::LeftStickButton; break;
		case 9: gcis = Src::RightStickButton; break;
		case 10: gcis = Src::LeftShoulder; break;
		case 11: gcis = Src::RightShoulder; break;
		case 12: gcis = Src::DPadUp; break;
		case 13: gcis = Src::DPadDown; break;
		case 14: gcis = Src::DPadLeft; break;
		case 15: gcis = Src::DPadRight; break;
		case 16: gcis = Src::Misc1; break;
		case 17: gcis = Src::Paddle1; break;
		case 18: gcis = Src::Paddle2; break;
		case 19: gcis = Src::Paddle3; break;
		case 20: gcis = Src::Paddle4; break;
		case 21: gcis = Src::TouchPad; break;
		case 22: gcis = Src::LeftStickAxis; break;
		case 23: gcis = Src::RightStickAxis; break;
		case 24: gcis = Src::LeftTrigger; break;
		case 25: gcis = Src::RightTrigger; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(MouseInputSource& mis, const char* label)
{
	using Src = MouseInputSource;

	static constexpr const char* kNames[] = {
		"Invalid",
		"Cursor",
		"LeftButton",
		"MiddleButton",
		"RightButton",
		"X1",
		"X2",
		"Wheel"
	};
	int cur = mis == Src::Invalid ? 0 :
			  mis == Src::Cursor ? 1 :
			  mis == Src::LeftButton ? 2 :
			  mis == Src::MiddleButton ? 3 :
			  mis == Src::RightButton ? 4 :
			  mis == Src::X1 ? 5 :
			  mis == Src::X2 ? 6 : 7;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: mis = MouseInputSource::Invalid; break;
		case 1: mis = MouseInputSource::Cursor; break;
		case 2: mis = MouseInputSource::LeftButton; break;
		case 3: mis = MouseInputSource::MiddleButton; break;
		case 4: mis = MouseInputSource::RightButton; break;
		case 5: mis = MouseInputSource::X1; break;
		case 6: mis = MouseInputSource::X2; break;
		case 7: mis = MouseInputSource::Wheel; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(SDL_MouseWheelDirection& mwd, const char* label)
{
	static constexpr const char* kNames[] = {
		"SDL_MOUSEWHEEL_NORMAL",
		"SDL_MOUSEWHEEL_FLIPPED"
	};
	int cur = mwd == SDL_MOUSEWHEEL_NORMAL ? 0 : 1;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: mwd = SDL_MOUSEWHEEL_NORMAL; break;
		case 1: mwd = SDL_MOUSEWHEEL_FLIPPED; break;
		}

		return true;
	}
	return false;
}


bool GuiEdit(GameControllerInputFieldValue& gcifv, const char* label)
{
	return GuiEditClass(gcifv, label, [](auto& gcifv) {
		bool b = GuiEdit(gcifv.trigger, "trigger");
		b |= GuiEdit(gcifv.axis, "axis");
		return b;
	});
}

bool GuiEdit(MouseInputValues::CursorValue& mivcv, const char* label)
{
	return GuiEditClass(mivcv, label, [](auto& mivcv) {
		bool b = GuiEdit(mivcv.absolutePos, "absolutePos");
		b |= GuiEdit(mivcv.relativePos, "relativePos");
		return b;
	});
}

bool GuiEdit(MouseInputValues::WheelValue& mivwv, const char* label)
{
	return GuiEditClass(mivwv, label, [](auto& mivwv) {
		bool b = GuiEdit(mivwv.scroll, "scroll");
		b |= GuiEdit(mivwv.direction, "direction");
		return b;
	});
}

bool GuiEdit(MouseInputValues& miv, const char* label)
{
	return GuiEditClass(miv, label, [](auto& miv) {
		bool b = GuiEdit(miv.cursor, "cursor");
		b |= GuiEdit(miv.wheel, "wheel");
		return b;
	});
}

bool GuiEdit(GameControllerInputMap& map, const char* label)
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();

	bool changed = false;

	for (size_t i = enum_start_v<GameControllerInputSource>; i < enum_size_v<GameControllerInputSource>; i++)
	{
		const auto k = static_cast<GameControllerInputSource>(i);

		ImGui::PushID(i++);

		ImGui::Text("%s : ", ToString(k));
		ImGui::SameLine();

		auto& val = map[k];
		changed |= GuiEdit(val);

		ImGui::PopID();
	}

	ImGui::Unindent();

	return changed;
}

bool GuiEdit(MouseInputMap& map, const char* label)
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();

	bool changed = false;

	for (size_t i = enum_start_v<MouseInputSource>; i < enum_size_v<MouseInputSource>; i++)
	{
		const auto k = static_cast<MouseInputSource>(i);

		ImGui::PushID(i++);

		ImGui::Text("%s : ", ToString(k));
		ImGui::SameLine();

		auto& val = map[k];
		changed |= GuiEdit(val);

		ImGui::PopID();
	}

	ImGui::Unindent();

	return changed;
}

bool GuiEdit(GameControllerInputField& gcif, const char* label)
{
	return GuiEditClass(gcif, label, [](auto& gcif) {
		bool b = GuiEdit(gcif.source, "source");
		b |= GuiEdit(gcif.state, "state");
		b |= GuiEdit(gcif.stateDuration, "stateDuration");
		b |= GuiEdit(gcif.value, "value");
		return b;
	});
}
bool GuiEdit(MouseInputField& mif, const char* label)
{
	return GuiEditClass(mif, label, [](auto& mif) {
		bool b = GuiEdit(mif.source, "source");
		b |= GuiEdit(mif.state, "state");
		b |= GuiEdit(mif.stateDuration, "stateDuration");
		return b;
	});
}

bool GuiEdit(GameControllerState& gcs)
{
	ImGui::PushID("GameControllerState");
	const bool changed = GuiEditClass(gcs, "GameControllerState", [](auto& gcs) {
		ImGui::LabelText("joystickID", "%d", gcs.joystickID);
		GuiEdit(gcs.inputs, "inputs");

		return false;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(MouseState& ms)
{
	ImGui::PushID("MouseState");
	const bool changed = GuiEditClass(ms, "MouseState", [](auto& ms) {
		bool b = GuiEdit(ms.inputs, "inputs");
		b |= GuiEdit(ms.values, "values");
		return b;
	});
	ImGui::PopID();
	return changed;
}

// GUI EDIT PROPERTY //

bool GuiEditProperty(InputState& is)
{
	static constexpr const char* kNames[] = {
		"None",
		"Pressed",
		"Released",
		"Held"
	};
	int cur = is == InputState::None ? 0 :
		is == InputState::Pressed ? 1 :
		is == InputState::Released ? 2 : 3;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: is = InputState::None; break;
		case 1: is = InputState::Pressed; break;
		case 2: is = InputState::Released; break;
		case 3: is = InputState::Held; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(GameControllerInputSource& gcis)
{
	using Src = GameControllerInputSource;

	static constexpr const char* kNames[] = {
		"Invalid",
		"A",
		"B",
		"X",
		"Y",
		"Back",
		"Guide",
		"Start",
		"LeftStickButton",
		"RightStickButton",
		"LeftShoulder",
		"RightShoulder",
		"DPadUp",
		"DPadDown",
		"DPadLeft",
		"DPadRight",
		"Misc1",
		"Paddle1",
		"Paddle2",
		"Paddle3",
		"Paddle4",
		"TouchPad",
		"LeftStickAxis",
		"RightStickAxis",
		"LeftTrigger",
		"RightTrigger"
	};
	int cur = gcis == Src::Invalid ? 0 :
		gcis == Src::A ? 1 :
		gcis == Src::B ? 2 :
		gcis == Src::X ? 3 :
		gcis == Src::Y ? 4 :
		gcis == Src::Back ? 5 :
		gcis == Src::Guide ? 6 :
		gcis == Src::Start ? 7 :
		gcis == Src::LeftStickButton ? 8 :
		gcis == Src::RightStickButton ? 9 :
		gcis == Src::LeftShoulder ? 10 :
		gcis == Src::RightShoulder ? 11 :
		gcis == Src::DPadUp ? 12 :
		gcis == Src::DPadDown ? 13 :
		gcis == Src::DPadLeft ? 14 :
		gcis == Src::DPadRight ? 15 :
		gcis == Src::Misc1 ? 16 :
		gcis == Src::Paddle1 ? 17 :
		gcis == Src::Paddle2 ? 18 :
		gcis == Src::Paddle3 ? 19 :
		gcis == Src::Paddle4 ? 20 :
		gcis == Src::TouchPad ? 21 :
		gcis == Src::LeftStickAxis ? 22 :
		gcis == Src::RightStickAxis ? 23 :
		gcis == Src::LeftTrigger ? 24 : 25;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: gcis = Src::Invalid; break;
		case 1: gcis = Src::A; break;
		case 2: gcis = Src::B; break;
		case 3: gcis = Src::X; break;
		case 4: gcis = Src::Y; break;
		case 5: gcis = Src::Back; break;
		case 6: gcis = Src::Guide; break;
		case 7: gcis = Src::Start; break;
		case 8: gcis = Src::LeftStickButton; break;
		case 9: gcis = Src::RightStickButton; break;
		case 10: gcis = Src::LeftShoulder; break;
		case 11: gcis = Src::RightShoulder; break;
		case 12: gcis = Src::DPadUp; break;
		case 13: gcis = Src::DPadDown; break;
		case 14: gcis = Src::DPadLeft; break;
		case 15: gcis = Src::DPadRight; break;
		case 16: gcis = Src::Misc1; break;
		case 17: gcis = Src::Paddle1; break;
		case 18: gcis = Src::Paddle2; break;
		case 19: gcis = Src::Paddle3; break;
		case 20: gcis = Src::Paddle4; break;
		case 21: gcis = Src::TouchPad; break;
		case 22: gcis = Src::LeftStickAxis; break;
		case 23: gcis = Src::RightStickAxis; break;
		case 24: gcis = Src::LeftTrigger; break;
		case 25: gcis = Src::RightTrigger; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(MouseInputSource& mis)
{
	using Src = MouseInputSource;

	static constexpr const char* kNames[] = {
		"Invalid",
		"Cursor",
		"LeftButton",
		"MiddleButton",
		"RightButton",
		"X1",
		"X2",
		"Wheel"
	};
	int cur = mis == Src::Invalid ? 0 :
		mis == Src::Cursor ? 1 :
		mis == Src::LeftButton ? 2 :
		mis == Src::MiddleButton ? 3 :
		mis == Src::RightButton ? 4 :
		mis == Src::X1 ? 5 :
		mis == Src::X2 ? 6 : 7;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: mis = MouseInputSource::Invalid; break;
		case 1: mis = MouseInputSource::Cursor; break;
		case 2: mis = MouseInputSource::LeftButton; break;
		case 3: mis = MouseInputSource::MiddleButton; break;
		case 4: mis = MouseInputSource::RightButton; break;
		case 5: mis = MouseInputSource::X1; break;
		case 6: mis = MouseInputSource::X2; break;
		case 7: mis = MouseInputSource::Wheel; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(SDL_MouseWheelDirection& mwd)
{
	static constexpr const char* kNames[] = {
		"SDL_MOUSEWHEEL_NORMAL",
		"SDL_MOUSEWHEEL_FLIPPED"
	};
	int cur = mwd == SDL_MOUSEWHEEL_NORMAL ? 0 : 1;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: mwd = SDL_MOUSEWHEEL_NORMAL; break;
		case 1: mwd = SDL_MOUSEWHEEL_FLIPPED; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(GameControllerInputFieldValue& gcifv)
{
	bool b = Property("trigger", gcifv.trigger);
	b |= Property("axis", gcifv.axis);
	return b;
}

bool GuiEditProperty(MouseInputValues::CursorValue& mivcv)
{
	bool b = Property("absolutePos", mivcv.absolutePos);
	b |= Property("relativePos", mivcv.relativePos);
	return b;
}

bool GuiEditProperty(MouseInputValues::WheelValue& mivwv)
{
	bool b = Property("scroll", mivwv.scroll);
	b |= Property("direction", mivwv.direction);
	return b;
}

bool GuiEditProperty(MouseInputValues& miv)
{
	bool b = PropertyGroup("cursor", [&miv] { return Property("", miv.cursor); });
	b |= PropertyGroup("wheel", [&miv] { return Property("", miv.wheel); });
	return b;
}

bool GuiEditProperty(GameControllerInputMap& map)
{
	bool changed = false;
	for (size_t i = enum_start_v<GameControllerInputSource>; i < enum_size_v<GameControllerInputSource>; i++)
	{
		const auto k = static_cast<GameControllerInputSource>(i);
		ImGui::PushID(i++);
		ImGui::Text("%s : ", ToString(k));
		ImGui::SameLine();
		auto& val = map[k];
		changed |= GuiEditProperty(val);
		ImGui::PopID();
	}
	return changed;
}

bool GuiEditProperty(MouseInputMap& map)
{
	bool changed = false;
	for (size_t i = enum_start_v<MouseInputSource>; i < enum_size_v<MouseInputSource>; i++)
	{
		const auto k = static_cast<MouseInputSource>(i);
		ImGui::PushID(i++);
		ImGui::Text("%s : ", ToString(k));
		ImGui::SameLine();
		auto& val = map[k];
		changed |= GuiEditProperty(val);
		ImGui::PopID();
	}
	return changed;
}

bool GuiEditProperty(GameControllerInputField& gcif)
{
	bool b = Property("source", gcif.source);
	b |= Property("state", gcif.state);
	b |= Property("stateDuration", gcif.stateDuration);
	b |= PropertyGroup("value", [&gcif] { return Property("", gcif.value); });
	return b;
}

bool GuiEditProperty(MouseInputField& mif)
{
	bool b = Property("source", mif.source);
	b |= Property("state", mif.state);
	b |= Property("stateDuration", mif.stateDuration);
	return b;
}

bool GuiEditProperty(GameControllerState& gcs)
{
	bool b = Property("joystickID", gcs.joystickID);
	b |= PropertyGroup("inputs", [&gcs] { return Property("", gcs.inputs); });
	return b;
}

bool GuiEditProperty(MouseState& ms)
{
	bool b = PropertyGroup("inputs", [&ms] { return Property("", ms.inputs); });
	b |= PropertyGroup("values", [&ms] { return Property("", ms.values); });
	return b;
}

} // ui

#endif