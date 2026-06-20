#include "GuiEditCore.h"

#if IMGUI_ENABLED

namespace ui {

bool GuiEdit(bool& b, const char* label)
{
	return ImGui::Checkbox(label, &b);
}

bool GuiEdit(int& i, const char* label)
{
	return ImGui::DragInt(label, &i);
}

bool GuiEdit(uint8_t& i, const char* label)
{
	int v = i;
	if (ImGui::DragInt(label, &v, 1.0f, 0, 255))
	{
		i = static_cast<uint8_t>(v);
		return true;
	}

	return false;
}

bool GuiEdit(size_t& si, const char* label)
{
	int v = si;
	if (ImGui::DragInt(label, &v))
	{
		si = static_cast<size_t>(v);
		return true;
	}

	return false;
}

bool GuiEdit(float& f, const char* label)
{
	return ImGui::DragFloat(label, &f);
}

bool GuiEdit(std::string& s, const char* label)
{
	assert(s.size() < 256);

	char buf[256];
	std::snprintf(buf, sizeof(buf), "%s", s.c_str());

	return ImGui::InputText(label, buf, sizeof(buf));
}

bool GuiEdit(SDL_Point& p, const char* label)
{
	int v[2] = { p.x, p.y };

	if (ImGui::DragInt2(label, v))
	{
		p.x = v[0];
		p.y = v[1];
		return true;
	}
	return false;
}

bool GuiEdit(SDL_FPoint& p, const char* label)
{
	float v[2] = { p.x, p.y };

	if (ImGui::DragFloat2(label, v))
	{
		p.x = v[0];
		p.y = v[1];
		return true;
	}
	return false;
}

bool GuiEdit(SDL_Rect& r, const char* label)
{
	return GuiEdit(r.x, "x") || GuiEdit(r.y, "y") || GuiEdit(r.w, "w") || GuiEdit(r.h, "h");
}

bool GuiEdit(SDL_FRect& r, const char* label)
{
	return GuiEdit(r.x, "x") || GuiEdit(r.y, "y") || GuiEdit(r.w, "w") || GuiEdit(r.h, "h");
}

std::string GuiGetEntityString(Entity_t e)
{
	return (e == kInvalidEntity) ? "<invalid>" : std::to_string(e);
}

} // ui

#endif