#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <SDL_Rect.h>
#include <optional>
#include <string>
#include "../../../core/commonObjects.h"
#include "../../../ecs/EntityT.h"

namespace ui {


template <typename...Args>
inline void GuiDraw(std::string_view frmt, Args&&...args)
{
	const std::string txt = std::format(frmt, std::forward<Args>(args)...);

	return ImGui::LabelText(txt.c_str());
}

/** Basic Data Types */
bool GuiEdit(bool& b, const char* label = "");
bool GuiEdit(int& i, const char* label = "");
bool GuiEdit(uint8_t& i, const char* label = "");
bool GuiEdit(size_t& si, const char* label = "");
bool GuiEdit(float& f, const char* label = "");
bool GuiEdit(std::string& s, const char* label = "");

/** SDL Data Types */
bool GuiEdit(SDL_Point& p, const char* label = "");
bool GuiEdit(SDL_FPoint& p, const char* label = "");
bool GuiEdit(SDL_Rect& r, const char* label = "");
bool GuiEdit(SDL_FRect& r, const char* label = "");


template <typename C> requires requires(C& c) { 
	{ c.begin() } -> std::same_as<typename C::iterator>;
	{ c.end() } -> std::same_as<typename C::iterator>;
}
inline bool GuiEditContainer(C& c, const char* label = "")
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();

	bool changed = false;
	int i = 0;
	for (auto& elem : c)
	{
		ImGui::PushID(i++);

		changed |= GuiEdit(elem);

		ImGui::PopID();
	}

	ImGui::Unindent();
}

template <typename C> requires requires(const C& c) {
	{ c.begin() } -> std::same_as<typename C::const_iterator>;
	{ c.end() } -> std::same_as<typename C::const_iterator>;
}
inline void GuiDrawContainer(const C& c, const char* label = "")
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();

	int i = 0;
	for (auto& elem : c)
	{
		ImGui::PushID(i++);

		GuiDraw("{}: {}", i, elem);

		ImGui::PopID();
	}

	ImGui::Unindent();
}

template <typename Class, typename Fn>
	requires std::is_invocable_r_v<bool, Fn, Class&>
inline bool GuiEditClass(Class& cl, const char* label, Fn&& fn)
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();
	
	const bool changed = std::invoke(fn, cl);
	
	ImGui::Unindent();
	
	return changed;
}

/** Core Engine Basic Data Types */
template <typename T>
inline bool GuiEdit(Dimensions<T>& dims, const char* label = "")
{
	return GuiEditClass(dims, label, [](auto& d) {
		return GuiEdit(dims.w, "w") || GuiEdit(dims.h, "h");
	});
}

/** Core std Basic Data Types */
template <typename T>
inline bool GuiEdit(std::optional<T>& op, const char* label = "")
{
	return GuiEditClass(op, label, [](auto& op) {
		bool hasValue = op.has_value();
		bool changed = false;
		if (ImGui::Checkbox(label, &hasValue))
		{
			if (hasValue && !op.has_value())
			{
				op.emplace();
				changed = true;
			}
			else if (!hasValue && op.has_value())
			{
				op.reset();
				changed = true;
			}
		}
		if (hasValue)
		{
			changed = GuiEdit(*op);			
		}

		return changed;
	});
}

/** Utils */
std::string GuiGetEntityString(Entity_t e);


} // ui


#endif