#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <format>
#include "../../ecs/Ecs.h"
#include "../../core/FixedString.h"

namespace test {


//inline std::string GuiGetEntityString(Entity_t e)
//{
//	return (e == kInvalidEntity) ? "<invalid>" : std::to_string(e);
//}
//
//
//template <typename...Args>
//inline void GuiDraw(std::string_view frmt, Args&&...args)
//{
//	const std::string txt = std::format(frmt, std::forward<Args>(args)...);
//
//	return ImGui::LabelText(txt.c_str());
//}
//
//inline bool GuiEdit(bool& b, const char* label = "")
//{
//	return ImGui::Checkbox(label, &b);
//}
//
//inline bool GuiEdit(int& i, const char* label = "")
//{
//	return ImGui::DragInt(label, &i);
//}
//
//inline bool GuiEdit(uint8_t& i, const char* label = "")
//{
//	int v = i;
//	if (ImGui::DragInt(label, &v, 1.0f, 0, 255))
//	{
//		i = static_cast<uint8_t>(v);
//		return true;
//	}
//
//	return false;
//}
//inline bool GuiEdit(size_t& si, const char* label = "")
//{
//	int v = si;
//	if (ImGui::DragInt(label, &v))
//	{
//		si = static_cast<size_t>(v);
//		return true;
//	}
//
//	return false;
//}
//
//inline bool GuiEdit(float& f, const char* label = "")
//{
//	return ImGui::DragFloat(label, &f);
//}
//
//inline bool GuiEdit(SDL_Point& p, const char* label = "")
//{
//	int v[2] = { p.x, p.y };
//
//	if (ImGui::DragInt2(label, v))
//	{
//		p.x = v[0];
//		p.y = v[1];
//		return true;
//	}
//	return false;
//}
//
//inline bool GuiEdit(SDL_FPoint& p, const char* label = "")
//{
//	float v[2] = { p.x, p.y };
//
//	if (ImGui::DragFloat2(label, v))
//	{
//		p.x = v[0];
//		p.y = v[1];
//		return true;
//	}
//	return false;
//}
//
//inline bool GuiEdit(SDL_Rect& r, const char* label = "")
//{
//	return GuiEdit(r.x, "x") || GuiEdit(r.y, "y") || GuiEdit(r.w, "w") || GuiEdit(r.h, "h");
//}
//inline bool GuiEdit(SDL_FRect& r, const char* label = "")
//{
//	return GuiEdit(r.x, "x") || GuiEdit(r.y, "y") || GuiEdit(r.w, "w") || GuiEdit(r.h, "h");
//}
//
//inline bool GuiEdit(std::string& s, const char* label = "")
//{
//	assert(s.size() < 256);
//	
//	char buf[256];
//	std::snprintf(buf, sizeof(buf), "%s", s.c_str());
//	
//	return ImGui::InputText(label, buf, sizeof(buf));
//}
//
//template <typename C> requires requires(C& c) { 
//	{ c.begin() } -> std::same_as<typename C::iterator>;
//	{ c.end() } -> std::same_as<typename C::iterator>;
//}
//inline bool GuiEditContainer(C& c, const char* label = "")
//{
//	ImGui::TextUnformatted(label);
//	ImGui::Indent();
//
//	bool changed = false;
//	int i = 0;
//	for (auto& elem : c)
//	{
//		ImGui::PushID(i++);
//
//		changed |= GuiEdit(elem);
//
//		ImGui::PopID();
//	}
//
//	ImGui::Unindent();
//}
//
//template <typename C> requires requires(const C& c) {
//	{ c.begin() } -> std::same_as<typename C::const_iterator>;
//	{ c.end() } -> std::same_as<typename C::const_iterator>;
//}
//inline void GuiDrawContainer(const C& c, const char* label = "")
//{
//	ImGui::TextUnformatted(label);
//	ImGui::Indent();
//
//	int i = 0;
//	for (auto& elem : c)
//	{
//		ImGui::PushID(i++);
//
//		GuiDraw("{}: {}", i, elem);
//
//		ImGui::PopID();
//	}
//
//	ImGui::Unindent();
//}
//
//template <typename Class, typename Fn>
//	requires std::is_invocable_r_v<bool, Fn, Class&>
//inline bool GuiEditClass(Class& cl, const char* label, Fn&& fn)
//{
//	ImGui::TextUnformatted(label);
//	ImGui::Indent();
//	
//	const bool changed = std::invoke(fn, cl);
//	
//	ImGui::Unindent();
//	
//	return changed;
//}
//
//template <typename T>
//inline bool GuiEdit(Dimensions<T>& dims, const char* label = "")
//{
//	return GuiEditClass(dims, label, [](auto& d) {
//		return GuiEdit(dims.w, "w") || GuiEdit(dims.h, "h");
//	});
//}
//
//inline bool GuiEdit(Transform& tf)
//{
//	return GuiEditClass(tf, "Transform", [](auto& tf) {
//		return GuiEdit(tf.position, "position") ||
//			   GuiEdit(tf.rotation, "rotation") ||
//			   GuiEdit(tf.scale, "scale");
//	});
//}
//
//inline bool GuiEdit(GlyphTextWriter& wr, const char* label = "")
//{
//	return GuiEditClass(wr, label, [](auto& wr) {
//		GuiDraw("resourceHandle: {}", wr.resourceHandle.GetHash());
//		return GuiEdit(wr.text);
//	});
//}
//
//inline bool GuiEdit(TextAlign& ta, const char* label = "")
//{
//	static constexpr const char* kNames[] = { "Left", "Center", "Right" };
//	int cur = static_cast<int>(ta);
//
//	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
//	{
//		switch (cur)
//		{
//		case 0: ta = TextAlign::Left; break;
//		case 1: ta = TextAlign::Center; break;
//		case 2: ta = TextAlign::Right; break;
//		}
//
//		return true;
//	}
//	return false;
//}
//
//inline bool GuiEdit(Anchor& a, const char* label = "")
//{
//	static constexpr const char* kNames[] = { 
//		"Left", "Right", "Top", "Bottom", "CenterX", "CenterY", 
//		"TopLeft", "TopRight", "BottomLeft", "BottomRight", "Center"};
//	int cur = static_cast<int>(a);
//
//	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
//	{
//		switch (cur)
//		{
//		case 0: a = Anchor::Left; break;
//		case 1: a = Anchor::Right; break;
//		case 2: a = Anchor::Top; break;
//		case 3: a = Anchor::Bottom; break;
//		case 4: a = Anchor::CenterX; break;
//		case 5: a = Anchor::CenterY; break;
//		case 6: a = Anchor::TopLeft; break;
//		case 7: a = Anchor::TopRight; break;
//		case 8: a = Anchor::BottomLeft; break;
//		case 9: a = Anchor::BottomRight; break;
//		case 10: a = Anchor::Center; break;
//		}
//
//		return true;
//	}
//	return false;
//}
//
//inline bool GuiEdit(TextFormatting& f, const char* label = "")
//{
//	return GuiEditClass(f, label, [](auto& f) {
//		return GuiEdit(f.bounds, "bounds") ||
//			   GuiEdit(f.align, "align") ||
//			   GuiEdit(f.letterSpacing, "letterSpacing") ||
//			   GuiEdit(f.scaleToBounds, "scaleToBounds");
//	});
//}
//
//inline bool GuiEdit(RGB& c, const char* label = "")
//{
//	return GuiEditClass(c, label, [](auto& c) {
//		return GuiEdit(c.r, "r") || GuiEdit(c.g, "g") || GuiEdit(c.b);
//	});
//}
//
//inline bool GuiEdit(SDL_BlendMode b, const char* label = "")
//{
//	static constexpr const char* kNames[] = {
//	    "SDL_BLENDMODE_NONE",
//	    "SDL_BLENDMODE_BLEND",
//	    "SDL_BLENDMODE_ADD",
//	    "SDL_BLENDMODE_MOD",
//	    "SDL_BLENDMODE_MUL"
//	};
//	int cur = static_cast<int>(b);
//
//	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
//	{
//		switch (cur)
//		{
//		case 0: b = SDL_BLENDMODE_NONE; break;
//		case 1: b = SDL_BLENDMODE_BLEND; break;
//		case 2: b = SDL_BLENDMODE_ADD; break;
//		case 3: b = SDL_BLENDMODE_MOD; break;
//		case 4: b = SDL_BLENDMODE_MUL; break;
//		}
//
//		return true;
//	}
//	return false;
//}
//
//inline bool GuiEdit(SDL_RendererFlip& f, const char* label = "")
//{
//	static constexpr const char* kNames[] = {
//		"SDL_FLIP_NONE",
//		"SDL_FLIP_HORIZONTAL",
//		"SDL_FLIP_VERTICAL"
//	};
//	int cur = static_cast<int>(f);
//
//	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
//	{
//		switch (cur)
//		{
//		case 0: f = SDL_FLIP_NONE; break;
//		case 1: f = SDL_FLIP_HORIZONTAL; break;
//		case 2: f = SDL_FLIP_VERTICAL; break;
//		}
//
//		return true;
//	}
//	return false;
//}
//
//inline bool GuiEdit(TextureMods& tm, const char* label = "")
//{
//	return GuiEditClass(tm, label, [](auto& tm) {
//		return GuiEdit(tm.color, "color") ||
//			   GuiEdit(tm.alpha, "alpha") ||
//			   GuiEdit(tm.blend, "blend");
//	});
//}
//
//inline bool GuiEdit(SDL_Color& c, const char* label = "")
//{
//	return GuiEditClass(c, label, [](auto& c) {
//		return GuiEdit(c.r, "r") || GuiEdit(c.g, "g") || GuiEdit(c.b) || GuiEdit(c.a);
//	});
//}
//
//inline bool GuiEdit(DebugDraw& dd, const char* label = "")
//{
//	return GuiEditClass(dd, label, [](auto& dd) {
//		return GuiEdit(dd.on, "on") || GuiEdit(dd.color, "color");
//	});
//}
//
//inline bool GuiEdit(DebugDrawSet& dds, const char* label = "")
//{
//	return GuiEditClass(dds, label, [](auto& dds) {
//		return GuiEdit(dds.boundingBox, "boundingBox") || 
//			   GuiEdit(dds.collider, "collider");
//	});
//}
//
//inline bool GuiEdit(RenderProfile::Anchors& as, const char* label = "")
//{
//	return GuiEditClass(as, label, [](auto& as) {
//		return GuiEdit(as.scale, "scale") || GuiEdit(as.rotation, "rotation");
//	});
//}
//
//inline bool GuiEdit(RenderProfile& rp, const char* label = "")
//{
//	return GuiEditClass(rp, label, [](auto& rp) {
//		return GuiEdit(rp.drawOrder, "drawOrder") ||
//			   GuiEdit(rp.mods, "mods") ||
//			   GuiEdit(rp.flip, "flip") ||
//			   GuiEdit(rp.offset, "offset") ||
//			   GuiEdit(rp.debugDraw, "debugDraw") ||
//			   GuiEdit(rp.isOverlay, "isOverlay") ||
//			   GuiEdit(rp.parallaxFactor, "parallaxFactor") ||
//			   GuiEdit(rp.anchor, "anchors");
//	});
//}
//
//inline bool GuiEdit(TextRenderableComponent& trc)
//{
//	return GuiEditClass(trc, "TextRenderableComponent", [](auto& trc) {
//		return GuiEdit(trc.writer, "writer") ||
//			   GuiEdit(trc.formatting, "formatting") ||
//			   GuiEdit(trc.profile, "renderProfile");
//	});
//}
//
//inline bool GuiEdit(AtlasPlot& ap, const char* label = "")
//{
//	return GuiEditClass(ap, label, [](auto& ap) {
//		return GuiEdit(ap.rect, "rect") || GuiEdit(ap.rotation, "rotation");
//	});
//}
//
//inline bool GuiEdit(Sprite& sp, const char* label = "")
//{
//	return GuiEditClass(sp, label, [](auto& sp) {
//		GuiDraw("resourceHandle: {}", sp.resourceHandle.GetHash());
//		return GuiEdit(sp.plot, "plot");
//	});
//}
//
//inline bool GuiEdit(CameraTarget& ct)
//{ 
//	return GuiEditClass(ct, "CameraTarget", [](auto& ct) {
//		return GuiEdit(ct.offset, "offset") || 
//			   GuiEdit(ct.followSpeed, "followSpeed") ||
//			   GuiEdit(ct.stopRadius, "stopRadius");
//	});
//}
//
//inline bool GuiEdit(Parent& p)
//{
//	return GuiEditClass(p, "Parent", [](auto& p) {	
//		GuiDraw("entityId: {}", GuiGetEntityString(p.entityId));
//		return false;
//	});
//}
//
//inline bool GuiEdit(Children& ch)
//{
//	return GuiEditClass(ch, "Children", [](auto& ch) {
//		const auto& es = ch.childEntityIds;
//		GuiDrawContainer(es, "childEntityIds");
//		return false;
//	});
//}


} // test

#endif