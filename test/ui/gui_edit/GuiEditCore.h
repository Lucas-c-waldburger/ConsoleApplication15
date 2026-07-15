#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <SDL_Rect.h>
#include <SDL_Pixels.h>
#include <optional>
#include <string>
#include <format>
#include "../GuiResource.h"
#include "../../../core/commonObjects.h"
#include "../../../core/FixedString.h"
#include "../../../core/Handle.h"
#include "../../../core/SizedEnumMap.h"
#include "../../../core/ReadOnly.h"
#include "../../../ecs/EntityT.h"
#include "../../../inputs/InputMap.h"


namespace ui {

template <FixedString...strs>
float GetFieldValueWidth()
{
	const ImGuiStyle& style = ImGui::GetStyle();

	//const float smallestTxtSize = []() {
	//	float smallest = std::numeric_limits<float>::max();
	//	((smallest = std::min(smallest, ImGui::CalcTextSize(strs).x)), ...);
	//	return smallest;
	//}();

	float labelWidth = 0.0f;
	((labelWidth += ImGui::CalcTextSize(strs).x), ...);
	//const float labelWidth = smallestTxtSize * sizeof...(strs);

	const float spacing = style.ItemSpacing.x * (sizeof...(strs) * 2 - 1);

	return (ImGui::GetContentRegionAvail().x - labelWidth - spacing) / sizeof...(strs);
}

/** GuiDraw */
template <typename T> requires requires(const T& t) {
	{ std::format("{}", t) } -> std::convertible_to<std::string>;
}
void GuiDraw(const T& val, const char* label = "")
{
	std::string txt;
	if (strlen(label) != 0)
	{
		txt = std::format("{} : {}", label, val);
	}
	else
	{
		txt = std::format("{}", val);
	}

	ImGui::TextUnformatted(txt.c_str());
}

template <typename T>
struct DragArgs
{
	float speed = 1.0f;
	T min = static_cast<T>(0);
	T max = static_cast<T>(0);
};

template <>
struct DragArgs<size_t>
{
	float speed = 1.0f;
	int min = 0;
	int max = 0;
};

namespace detail {
template <typename T>
struct is_drag_args : std::false_type {};
template <typename T>
struct is_drag_args<DragArgs<T>> : std::true_type {};

template <typename...Ts>
struct is_last_drag_args : std::false_type {};
template <typename T>
struct is_last_drag_args<T> : std::bool_constant<is_drag_args<T>::value> {};
} // detail

template <typename T>
inline constexpr bool is_last_drag_args_v = detail::is_last_drag_args<T>::value;

/** Basic Data Types */
bool GuiEdit(bool& b, const char* label = "");
bool GuiEdit(int& i, const char* label = "", DragArgs<int> args = {});
bool GuiEdit(uint8_t& i, const char* label = "", DragArgs<int> args = {});
bool GuiEdit(int16_t& i, const char* label = "", DragArgs<int> args = {});
bool GuiEdit(uint64_t& i, const char* label = "", DragArgs<int> args = {});
bool GuiEdit(size_t& si, const char* label = "", DragArgs<int> args = {});
bool GuiEdit(float& f, const char* label = "", DragArgs<float> args = {});
bool GuiEdit(std::string& s, const char* label = "");
bool GuiEdit(char& c, const char* label = "");

/** SDL Data Types */
bool GuiEdit(SDL_Point& p, const char* label = "", DragArgs<int> args = {});
bool GuiEdit(SDL_FPoint& p, const char* label = "", DragArgs<float> args = {});
bool GuiEdit(SDL_Rect& r, const char* label = "", DragArgs<int> args = {});
bool GuiEdit(SDL_FRect& r, const char* label = "", DragArgs<float> args = {});
bool GuiEdit(SDL_Color& c, const char* label = "");

/** STL Container Data Types */
template <typename C> requires requires(C& c) { 
	{ c.begin() } -> std::same_as<typename C::iterator>;
	{ c.end() } -> std::same_as<typename C::iterator>;
}
inline bool GuiEditContainer(C& c, const char* label = "")
{
	bool changed = false;

	if (ImGui::TreeNode(label))
	{
		ImGui::Indent();

		int i = 0;
		for (auto& elem : c)
		{
			ImGui::PushID(i++);

			changed |= GuiEdit(elem);

			ImGui::PopID();
		}

		ImGui::Unindent();
		ImGui::TreePop();
	}

	return changed;
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

		const std::string elemStr = std::format("{}", elem);

		ImGui::Text("%d: ", i);

		ImGui::PopID();
	}

	ImGui::Unindent();
}

template <typename Class, typename Fn>
	requires std::is_invocable_r_v<bool, Fn, Class&>
inline bool GuiEditClass(Class& cl, const char* label, Fn&& fn)
{
	//ImGui::TextUnformatted(label);
	//ImGui::Indent(); 
	bool changed = false;

	if (ImGui::TreeNode(label))
	{
		ImGui::Indent();

		changed = std::invoke(fn, cl);

		ImGui::Unindent();
		ImGui::TreePop();
	}
	
	//ImGui::Unindent();
	
	return changed;
}

/** Core Engine Basic Data Types */
template <typename T>
inline bool GuiEdit(Dimensions<T>& dims, const char* label = "")
{
	return GuiEditClass(dims, label, [](auto& d) {
		bool b = GuiEdit(d.w, "w");
		b |= GuiEdit(d.h, "h");
		return b;
	});
}
template <typename T>
inline bool GuiEdit(HandedPair<T>& hp, const char* label = "")
{
	return GuiEditClass(hp, label, [](auto& hp) {
		bool b = GuiEdit(hp.left, "left");
		b |= GuiEdit(hp.right, "right");
		return b;
	});
}
template <typename T>
inline bool GuiEdit(Range<T>& r, const char* label = "")
{
	return GuiEditClass(r, label, [](auto& r) {
		bool b = GuiEdit(r.min, "min");
		b |= GuiEdit(r.max, "max");
		return b;
	});
}

template <typename T>
inline void GuiDraw(const Handle<T>& h, const char* label = "")
{
	ImGui::PushID(h.GetHash());
	ImGui::LabelText(label, "%d", h.GetHash());
	ImGui::PopID();
}

template <SomeSizedEnum EnumKey, typename Value> requires
	requires (EnumKey k) { { ToString(k) } -> std::convertible_to<std::string_view>; }
inline bool GuiEdit(SizedEnumMap<EnumKey, Value>& map, const char* label = "")
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();

	bool changed = false;

	for (size_t i = enum_start_v<EnumKey>; i < enum_size_v<EnumKey>; i++)
	{
		const auto k = static_cast<EnumKey>(i);

		ImGui::PushID(i++);

		ImGui::Text("%s : ", ToString(k));
		ImGui::SameLine();

		auto& val = map[k];
		changed |= GuiEdit(val);

		ImGui::PopID();
	}

	ImGui::Unindent();
}

//template <typename T, typename U>
//inline bool GuiEdit(InputMap<T, U>& map, const char* label = "")
//{
//	ImGui::TextUnformatted(label);
//	ImGui::Indent();
//
//	bool changed = false;
//
//	for (size_t i = enum_start_v<T>; i < enum_size_v<T>; i++)
//	{
//		const auto k = static_cast<T>(i);
//
//		ImGui::PushID(i++);
//
//		ImGui::Text("%s : ", ToString(k));
//		ImGui::SameLine();
//
//		auto& val = map[k];
//		changed |= GuiEdit(val);
//
//		ImGui::PopID();
//	}
//
//	ImGui::Unindent();
//}

template <SomeSizedEnum EnumKey, typename Value> requires 
	requires (EnumKey k) { { ToString(k) } -> std::convertible_to<std::string_view>; }
inline void GuiDraw(const SizedEnumMap<EnumKey, Value>& map, const char* label = "")
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();

	for (size_t i = enum_start_v<EnumKey>; i < enum_size_v<EnumKey>; i++)
	{
		const auto k = static_cast<EnumKey>(i);

		ImGui::PushID(i++);

		const std::string txt = std::format("{} : {}", ToString(k), map[k]);
		ImGui::Text(txt.c_str());

		ImGui::PopID();
	}

	ImGui::Unindent();
}

template <typename T>
inline void GuiDraw(const ReadOnly<T>& ro, const char* label = "")
{
	GuiDraw(ro.GetData(), label);
}

/** Core std Basic Data Types */
template <typename T>
inline bool GuiEdit(std::optional<T>& op, const char* label = "")
{
	ImGui::TextUnformatted(label);
	ImGui::Indent();

	bool hasValue = op.has_value();
	bool changed = false;
	if (ImGui::Checkbox("has value", &hasValue))
	{
		if (hasValue && !op.has_value())
		{
			op = T{};
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
		T& val = *op;
		changed = GuiEdit(val);
	}

	ImGui::Unindent();

	return changed;
}

// GUI EDIT/DRAW PROPERTY //

template <typename T>
inline void GuiDrawProperty(const T& val)
{
	std::string txt = std::format("{}", val);
	ImGui::TextUnformatted(txt.c_str());
}

void GuiDrawProperty(const bool& b);
void GuiDrawProperty(const SDL_FRect& r);
void GuiDrawProperty(const SDL_FPoint& p);

/** defgroup Basic data types @{ */
template <typename T> requires std::is_integral_v<T>
inline bool GuiEditProperty(T& i, DragArgs<int> args = {})
{
	int v = static_cast<int>(i);
	if (ImGui::DragInt("##Value", &v, args.speed, args.min, args.max))
	{
		i = static_cast<T>(v);
		return true;
	}
	return false;
}
template <typename T> requires std::is_floating_point_v<T>
inline bool GuiEditProperty(T& f, DragArgs<float> args = {})
{
	float v = static_cast<float>(f);
	if (ImGui::DragFloat("##Value", &v, args.speed, args.min, args.max))
	{
		f = static_cast<T>(v);
		return true;
	}
	return false;
}

bool GuiEditProperty(bool& b);
bool GuiEditProperty(std::string& str);
bool GuiEditProperty(SDL_Point& p, DragArgs<int> args = {});
bool GuiEditProperty(SDL_FPoint& p, DragArgs<float> args = {});
bool GuiEditProperty(SDL_Rect& r, DragArgs<int> args = {});
bool GuiEditProperty(SDL_FRect& r, DragArgs<float> args = {});
bool GuiEditProperty(SDL_Color& c);

template <FixedString...strs, typename...Args> requires (sizeof...(strs) == sizeof...(Args))
inline bool GuiEditProperties(Args&...args)
{
	static constexpr auto draw = []<FixedString label>(float w, auto& arg, int itemTrack) {
		if (itemTrack > 1)
		{
			ImGui::SameLine();
		}

		//WithFont(GuiResource::Fonts().regular, [] {
			ImGui::TextUnformatted(label);
		//});

		ImGui::SameLine();

		if (itemTrack >= sizeof...(strs))
		{
			w = -FLT_MIN;
		}
		ImGui::SetNextItemWidth(w);

		ImGui::PushID(&arg);
		const bool changed = GuiEditProperty(arg);
		ImGui::PopID();

		return changed;
	};

	const float fieldWidth = GetFieldValueWidth<strs...>();
	bool changed = false;
	int itemTrack = 1;
	((changed |= draw.template operator()<strs>(fieldWidth, args, itemTrack++)), ...);

	return changed;
}

template <FixedString...strs, typename T, typename...Args> requires (sizeof...(strs) == sizeof...(Args))
inline bool GuiEditProperties(const DragArgs<T>& drag, Args&...args)
{
	static constexpr auto draw = []<FixedString label>(float w, auto& arg, const auto& drag, int itemTrack) {
		if (itemTrack > 1)
		{
			ImGui::SameLine();
		}

		WithFont(GuiResource::Fonts().regular, [] {
			ImGui::TextUnformatted(label);
		});

		ImGui::SameLine();

		if (itemTrack >= sizeof...(strs))
		{
			w = -FLT_MIN;
		}
		ImGui::SetNextItemWidth(w);

		ImGui::PushID(&arg);
		const bool changed = GuiEditProperty(arg, drag);
		ImGui::PopID();

		return changed;
	};

	const float fieldWidth = GetFieldValueWidth<strs...>();
	bool changed = false;
	int itemTrack = 1;
	((changed |= draw.template operator()<strs>(fieldWidth, args, drag, itemTrack++)), ...);

	return changed;
}

/** @} */

/** @defgroup Core Engine Basic Data Types @{ */
template <typename T>
inline bool GuiEditProperty(Dimensions<T>& d, DragArgs<T> args = {})
{
	return GuiEditProperties<"w", "h">(args, d.w, d.h);
}
template <typename T, typename...Args>
inline bool GuiEditProperty(HandedPair<T>& hp, Args&&...args)
{
	return GuiEditProperties<"left", "right">(std::forward<Args>(args)..., hp.left, hp.right);
}
template <typename T, typename...Args>
inline bool GuiEditProperty(Range<T>& r, Args&&...args)
{
	return GuiEditProperties<"min", "max">(std::forward<Args>(args)..., r.min, r.max);
}

bool GuiEditProperty(Range<SDL_FPoint>& r, DragArgs<float> args = {});
bool GuiEditProperty(Range<float>& f, DragArgs<float> args = {});

template <typename T>
inline void GuiDrawProperty(const Handle<T>& h)
{
	GuiDrawProperty(h.GetHash());
}
template <typename T>
inline void GuiDrawProperty(const ReadOnly<T>& ro)
{
	GuiDrawProperty(ro.GetData());
}
/** @} */

/** @defgroup Core Std Basic Data Types @{ */ 
template <typename T, typename...Args>
inline bool GuiEditProperty(std::optional<T>& op, Args&&...args)
{
	bool hasValue = op.has_value();
	bool changed = false;
	if (ImGui::Checkbox("##Op", &hasValue))
	{
		if (hasValue && !op.has_value())
		{
			op = T{};
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
		T& val = *op;
		changed = GuiEditProperty(val, std::forward<Args>(args)...);
	}

	return changed;
}

struct VecArgs
{
	size_t minSize = 0;
	size_t maxSize = std::numeric_limits<size_t>::max();
};

bool GuiEditProperty(std::vector<SDL_FPoint>& v, VecArgs args = {});

template <typename C, typename...Args> requires requires(C& c) {
	{ c.begin() } -> std::same_as<typename C::iterator>;
	{ c.end() } -> std::same_as<typename C::iterator>;
}
inline bool GuiEditProperty(C& c, Args&&...args)
{
	bool changed = false;

	int i = 0;
	for (auto& elem : c)
	{
		ImGui::PushID(i++);

		changed |= GuiEditProperty(elem, std::forward<Args>(args)...);

		ImGui::PopID();
	}

	return changed;
}
template <typename C> requires requires(const C& c) {
	{ c.begin() } -> std::same_as<typename C::const_iterator>;
	{ c.end() } -> std::same_as<typename C::const_iterator>;
}
inline void GuiDrawProperty(const C& c)
{
	int i = 0;
	for (const auto& elem : c)
	{
		ImGui::PushID(i++);

		GuiDrawProperty(elem);

		ImGui::PopID();
	}
}

void GuiDrawProperty(const std::string_view& sv);

/** @} */ 

/** Utils */
std::string GuiGetEntityString(Entity_t e);



} // ui


#endif