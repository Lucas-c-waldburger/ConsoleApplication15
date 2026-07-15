#include "GuiEditCore.h"

#if IMGUI_ENABLED
#include "GuiEditPropertyTable.h"

namespace ui {

namespace {

void BeginDisabledNoStyle(bool doDisable = true)
{
	ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
	ImGui::BeginDisabled(doDisable);
}

void EndDisabledNoStyle()
{
	ImGui::EndDisabled();
	ImGui::PopStyleVar();
}

} // unnamed

bool GuiEdit(bool& b, const char* label)
{
	return ImGui::Checkbox(label, &b);
}

bool GuiEdit(int& i, const char* label, DragArgs<int> args)
{
	return ImGui::DragInt(label, &i, args.speed, args.min, args.max);
}

bool GuiEdit(uint8_t& i, const char* label, DragArgs<int> args)
{
	int v = i;
	if (ImGui::DragInt(label, &v, args.speed, 0, 255))
	{
		i = static_cast<uint8_t>(v);
		return true;
	}

	return false;
}

bool GuiEdit(int16_t& i, const char* label, DragArgs<int> args)
{
	int v = i;
	if (ImGui::DragInt(label, &v, args.speed, args.min, args.max))
	{
		i = static_cast<int16_t>(v);
		return true;
	}

	return false;
}

bool GuiEdit(uint64_t& i, const char* label, DragArgs<int> args)
{
	int v = i;
	if (ImGui::DragInt(label, &v, args.speed, args.min, args.max))
	{
		i = static_cast<uint64_t>(v);
		return true;
	}

	return false;
}

bool GuiEdit(size_t& si, const char* label, DragArgs<int> args)
{
	int v = si;
	if (ImGui::DragInt(label, &v, args.speed, args.min, args.max))
	{
		si = static_cast<size_t>(v);
		return true;
	}

	return false;
}

bool GuiEdit(float& f, const char* label, DragArgs<float> args)
{
	return ImGui::DragFloat(label, &f, args.speed, args.min, args.max);
}

bool GuiEdit(std::string& s, const char* label)
{
	assert(s.size() < 256);

	char buf[256];
	std::snprintf(buf, sizeof(buf), "%s", s.c_str());
	
	if (ImGui::InputText(label, buf, sizeof(buf)))
	{
		s = buf;
		return true;
	}
	
	return false;
}


bool GuiEdit(char& c, const char* label)
{
	std::string asStr{ c };
	if (GuiEdit(asStr, label))
	{
		if (!asStr.empty())
		{
			c = asStr.front();
			return true;
		}
	}
	return false;
}


bool GuiEdit(SDL_Point& p, const char* label, DragArgs<int> args)
{
	int v[2] = { p.x, p.y };

	if (ImGui::DragInt2(label, v, args.speed, args.min, args.max))
	{
		p.x = v[0];
		p.y = v[1];
		return true;
	}
	return false;
}

bool GuiEdit(SDL_FPoint& p, const char* label, DragArgs<float> args)
{
	float v[2] = { p.x, p.y };

	if (ImGui::DragFloat2(label, v, args.speed, args.min, args.max))
	{
		p.x = v[0];
		p.y = v[1];
		return true;
	}
	return false;
}

bool GuiEdit(SDL_Rect& r, const char* label, DragArgs<int> args)
{
	bool b = GuiEdit(r.x, "x", args);
	b |= GuiEdit(r.y, "y", args);
	b |= GuiEdit(r.w, "w", args);
	b |= GuiEdit(r.h, "h", args);
	return b;
}

bool GuiEdit(SDL_FRect& r, const char* label, DragArgs<float> args)
{
	bool b = GuiEdit(r.x, "x", args);
	b |= GuiEdit(r.y, "y", args);
	b |= GuiEdit(r.w, "w", args);
	b |= GuiEdit(r.h, "h", args);
	return b;
}

bool GuiEdit(SDL_Color& c, const char* label)
{
	ImGui::PushID(&c);
	const bool changed = GuiEditClass(c, label, [](auto& c) {
		bool b = GuiEdit(c.r, "r");
		b |= GuiEdit(c.g, "g");
		b |= GuiEdit(c.b, "b");
		b |= GuiEdit(c.a, "a");
		return b;
	});
	ImGui::PopID();
	return changed;
}

void GuiDrawProperty(const bool& b)
{
	BeginDisabledNoStyle();

	bool bCpy = b;
	ImGui::Checkbox("##Value", &bCpy);

	EndDisabledNoStyle();
}

void GuiDrawProperty(const SDL_FRect& r)
{
	BeginDisabledNoStyle();

	SDL_FRect rCpy = r;
	GuiEditProperties<"X", "Y", "W", "H">(rCpy.x, rCpy.y, rCpy.w, rCpy.h);

	EndDisabledNoStyle();
}

void GuiDrawProperty(const SDL_FPoint& p)
{
	BeginDisabledNoStyle();

	SDL_FPoint pCpy = p;
	GuiEditProperties<"X", "Y">(pCpy.x, pCpy.y);

	EndDisabledNoStyle();
}

bool GuiEditProperty(Range<SDL_FPoint>& r, DragArgs<float> args)
{
	bool b = Property("min", r.min, args);
	b |= Property("max", r.max, args);
	return b;
}
bool GuiEditProperty(Range<float>& f, DragArgs<float> args)
{
	bool b = Property("min", f.min, args);
	b |= Property("max", f.max, args);
	return b;
}

bool GuiEditProperty(std::vector<SDL_FPoint>& v, VecArgs args)
{
	bool changed = false;
	size_t erasedIdx = std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < v.size(); ++i)
	{
		auto& p = v[i];

		ImGui::TableNextRow();

		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();

		if (gPropertyDepth > 0)
		{
			ImGui::Indent(gPropertyDepth * 12.0f);
		}

		if (gPropertyDepth > 0)
		{
			ImGui::Unindent(gPropertyDepth * 12.0f);
		}

		ImGui::PushID(i);

		float pv[2] = { p.x, p.y };

		if (ImGui::DragFloat2("##Value", pv))
		{
			p.x = pv[0];
			p.y = pv[1];
			changed |= true;
		}

		//changed |= GuiEditProperties<"X", "Y">(p.x, p.y);

		ImGui::TableNextColumn();

		ImGui::SetNextItemWidth(-FLT_MIN);

		const std::string xBtnLabel = std::format("x##{}", i);

		ImGui::BeginDisabled(v.size() < args.minSize);

		if (ImGui::Button(xBtnLabel.c_str()))
		{
			changed |= true;
			erasedIdx = i;
		}

		ImGui::EndDisabled();

		ImGui::PopID();
	}

	if (erasedIdx != std::numeric_limits<size_t>::max())
	{
		if (v.size() > args.minSize)
		{
			assert(erasedIdx < v.size());

			v.erase(v.begin() + erasedIdx);
		}
	}

	if (ImGui::Button("Add"))
	{
		if (v.size() < args.maxSize)
		{
			v.emplace_back(0.0f, 0.0f);
		}
	}

	return changed;
}

void GuiDrawProperty(const std::string_view& sv)
{
	ImGui::TextUnformatted(sv.data());
}

std::string GuiGetEntityString(Entity_t e)
{
	return (e == kInvalidEntity) ? "<invalid>" : std::to_string(e);
}

bool GuiEditProperty(bool& b)
{
	return ImGui::Checkbox("##Value", &b);
}

// GUI EDIT PROPERTY
bool GuiEditProperty(std::string& s)
{
	assert(s.size() < 256);

	char buf[256];
	std::snprintf(buf, sizeof(buf), "%s", s.c_str());

	if (ImGui::InputText("##Value", buf, sizeof(buf)))
	{
		s = buf;
		return true;
	}

	return false;
}

bool GuiEditProperty(SDL_Point& p, DragArgs<int> args)
{
	int v[2] = { p.x, p.y };

	if (ImGui::DragInt2("##Value", v, args.speed, args.min, args.max))
	{
		p.x = v[0];
		p.y = v[1];
		return true;
	}
	return false;
}

bool GuiEditProperty(SDL_FPoint& p, DragArgs<float> args)
{
	return GuiEditProperties<"X", "Y">(p.x, p.y);
}

bool GuiEditProperty(SDL_Rect& r, DragArgs<int> args)
{
	return GuiEditProperties<"X", "Y", "W", "H">(r.x, r.y, r.w, r.h);
}

bool GuiEditProperty(SDL_FRect& r, DragArgs<float> args)
{
	return GuiEditProperties<"X", "Y", "W", "H">(r.x, r.y, r.w, r.h);
}

bool GuiEditProperty(SDL_Color& c)
{
	//bool b = Property("r", c.r);
	//b |= Property("g", c.g);
	//b |= Property("b", c.b);
	//b |= Property("a", c.a);
	//return b;

	//int v[4] = { c.r, c.g, c.b, c.a };

	//if (ImGui::DragInt4("##Value", v))
	//{
	//	c.r = v[0];
	//	c.g = v[1];
	//	c.b = v[2];
	//	c.a = v[3];
	//	return true;
	//}
	//return false;

	//float v[4] = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };

	//if (ImGui::ColorEdit4("##Value", v, ImGuiColorEditFlags_Uint8))
	//{
	//	c.r = static_cast<Uint8>(v[0] * 255.0f);
	//	c.g = static_cast<Uint8>(v[1] * 255.0f);
	//	c.b = static_cast<Uint8>(v[2] * 255.0f);
	//	c.a = static_cast<Uint8>(v[3] * 255.0f);
	//	return true;
	//}
	//return false;

	DragArgs<int> drag{ 1.0f, 0, 255 };

	return GuiEditProperties<"R", "G", "B", "A">(drag, c.r, c.g, c.b, c.a);
}

} // ui

#endif