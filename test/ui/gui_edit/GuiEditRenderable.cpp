#include "GuiEditRenderable.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "GuiEditBasicComponents.h"
#include "GuiEditPropertyTable.h"

namespace ui {

bool GuiEdit(GlyphTextWriter& wr, const char* label)
{
	const bool changed = GuiEditClass(wr, label, [](auto& wr) {
		GuiDraw(wr.resourceHandle, "resourceHandle");
		return GuiEdit(wr.text, "text");
	});
	return changed;
}

bool GuiEdit(TextAlign& ta, const char* label)
{
	static constexpr const char* kNames[] = { 
		"Left", 
		"Center", 
		"Right" 
	};
	int cur = ta == TextAlign::Left ? 0 :
			  ta == TextAlign::Center ? 1 : 2;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: ta = TextAlign::Left; break;
		case 1: ta = TextAlign::Center; break;
		case 2: ta = TextAlign::Right; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(Anchor& a, const char* label)
{
	static constexpr const char* kNames[] = {
		"Left", 
		"Right", 
		"Top", 
		"Bottom", 
		"CenterX", 
		"CenterY",
		"TopLeft", 
		"TopRight", 
		"BottomLeft", 
		"BottomRight", 
		"Center" 
	};
	int cur = a == Anchor::Left ? 0 :
			  a == Anchor::Right ? 1 :
			  a == Anchor::Top ? 2 :
			  a == Anchor::Bottom ? 3 :
			  a == Anchor::CenterX ? 4 :
			  a == Anchor::CenterY ? 5 :
			  a == Anchor::TopLeft ? 6 :
			  a == Anchor::TopRight ? 7 :
			  a == Anchor::BottomLeft ? 8 :
			  a == Anchor::BottomRight ? 9 : 10;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: a = Anchor::Left; break;
		case 1: a = Anchor::Right; break;
		case 2: a = Anchor::Top; break;
		case 3: a = Anchor::Bottom; break;
		case 4: a = Anchor::CenterX; break;
		case 5: a = Anchor::CenterY; break;
		case 6: a = Anchor::TopLeft; break;
		case 7: a = Anchor::TopRight; break;
		case 8: a = Anchor::BottomLeft; break;
		case 9: a = Anchor::BottomRight; break;
		case 10: a = Anchor::Center; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(TextFormatting& f, const char* label)
{
	return GuiEditClass(f, label, [](auto& f) {
		bool b = GuiEdit(f.bounds, "bounds");
		b |= GuiEdit(f.align, "align");
		b |= GuiEdit(f.letterSpacing, "letterSpacing");
		b |= GuiEdit(f.scaleToBounds, "scaleToBounds");
		return b;
	});
}

bool GuiEdit(RGB& c, const char* label)
{
	return GuiEditClass(c, label, [](auto& c) {
		ImGui::PushID(&c);

		bool b = GuiEdit(c.r, "r");
		b |= GuiEdit(c.g, "g");
		b |= GuiEdit(c.b, "b");

		ImGui::PopID();
		return b;
	});
}

bool GuiEdit(SDL_BlendMode b, const char* label)
{
	static constexpr const char* kNames[] = {
		"SDL_BLENDMODE_NONE",
		"SDL_BLENDMODE_BLEND",
		"SDL_BLENDMODE_ADD",
		"SDL_BLENDMODE_MOD",
		"SDL_BLENDMODE_MUL"
	};
	int cur = b == SDL_BLENDMODE_NONE ? 0 :
		      b == SDL_BLENDMODE_BLEND ? 1 :
		      b == SDL_BLENDMODE_ADD ? 2 :
		      b == SDL_BLENDMODE_MOD ? 3 : 4;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: b = SDL_BLENDMODE_NONE; break;
		case 1: b = SDL_BLENDMODE_BLEND; break;
		case 2: b = SDL_BLENDMODE_ADD; break;
		case 3: b = SDL_BLENDMODE_MOD; break;
		case 4: b = SDL_BLENDMODE_MUL; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(SDL_RendererFlip& f, const char* label)
{
	static constexpr const char* kNames[] = {
		"SDL_FLIP_NONE",
		"SDL_FLIP_HORIZONTAL",
		"SDL_FLIP_VERTICAL"
	};
	int cur = f == SDL_FLIP_NONE ? 0 :
			  f == SDL_FLIP_HORIZONTAL ? 1 : 2;

	if (ImGui::Combo(label, &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: f = SDL_FLIP_NONE; break;
		case 1: f = SDL_FLIP_HORIZONTAL; break;
		case 2: f = SDL_FLIP_VERTICAL; break;
		}

		return true;
	}
	return false;
}

bool GuiEdit(TextureMods& tm, const char* label)
{
	return GuiEditClass(tm, label, [](auto& tm) {
		bool b = GuiEdit(tm.color, "color");
		b |= GuiEdit(tm.alpha, "alpha");
		b |= GuiEdit(tm.blend, "blend");
		return b;
	});
}


bool GuiEdit(DebugDraw& dd, const char* label)
{
	ImGui::PushID(&dd);
	const bool changed = GuiEditClass(dd, label, [](auto& dd) {
		bool b = GuiEdit(dd.on, "on");
		b |= GuiEdit(dd.color, "color");
		return b;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(DebugDrawSet& dds, const char* label)
{
	return GuiEditClass(dds, label, [](auto& dds) {
		bool b = GuiEdit(dds.boundingBox, "boundingBox");
		b |= GuiEdit(dds.collider, "collider");
		return b;
	});
}

bool GuiEdit(RenderProfile::Anchors& as, const char* label)
{
	return GuiEditClass(as, label, [](auto& as) {
		bool b = GuiEdit(as.scale, "scale");
		b |= GuiEdit(as.rotation, "rotation");
		return b;
	});
}

bool GuiEdit(RenderProfile& rp, const char* label)
{
	return GuiEditClass(rp, label, [](auto& rp) {
		bool b = GuiEdit(rp.drawOrder, "drawOrder");
		b |= GuiEdit(rp.mods, "mods");
		b |= GuiEdit(rp.flip, "flip");
		b |= GuiEdit(rp.offset, "offset");
		b |= GuiEdit(rp.debugDraw, "debugDraw");
		b |= GuiEdit(rp.isOverlay, "isOverlay");
		b |= GuiEdit(rp.parallaxFactor, "parallaxFactor");
		b |= GuiEdit(rp.anchor, "anchors");
		return b;
	});
}

bool GuiEdit(AtlasPlot& ap, const char* label)
{
	return GuiEditClass(ap, label, [](auto& ap) {
		bool b = GuiEdit(ap.rect, "rect");
		b |= GuiEdit(ap.rotation, "rotation");
		return b;
	});
}

bool GuiEdit(Sprite& sp, const char* label)
{
	return GuiEditClass(sp, label, [](auto& sp) {
		GuiDraw(sp.resourceHandle, "resourceHandle");
		return GuiEdit(sp.plot, "plot");
	});
}

bool GuiEdit(SpriteSeriesIndex& ssi, const char* label)
{
	return GuiEditClass(ssi, label, [](auto& ssi) {
		int cur = static_cast<int>(ssi.current);
		bool changed = false;
		if (ImGui::DragInt("current", &cur, 1.0f, 0, static_cast<int>(ssi.max)))
		{
			ssi.current = static_cast<size_t>(cur);
			changed = true;
		}
		ImGui::LabelText("max", "%d", ssi.max);
		return changed;
	});
}

bool GuiEdit(Glyph& g, const char* label)
{
	const bool changed = GuiEditClass(g, label, [](auto& g) {
		bool b = GuiEdit(g.character, "character");
		b |= GuiEdit(g.plot, "plot");
		b |= GuiEdit(g.advance, "advance");
		return b;
	});
	return changed;
}

bool GuiEdit(GlyphCacheData& gcd, const char* label)
{
	const bool changed = GuiEditClass(gcd, label, [](auto& gcd) {
		bool b = GuiEdit(gcd.glyph, "glyph");
		b |= GuiEdit(gcd.destRect, "destRect");
		b |= GuiEdit(gcd.rotationCenter, "rotationCenter");
		return b;
	});
	return changed;
}

bool GuiEdit(TextRenderableGlyphCache::CacheContext& ctx, const char* label)
{
	const bool changed = GuiEditClass(ctx, label, [](auto& ctx) {
		bool b = GuiEdit(ctx.transform);
		b |= GuiEdit(ctx.formatting, "formatting");
		b |= GuiEdit(ctx.offset, "offset");
		GuiDraw(ctx.resourceHandle, "resourceHandle");
		b |= GuiEdit(ctx.textHash, "textHash");
		return b;
	});
	return changed;
}

bool GuiEdit(TextRenderableComponent& trc)
{
	ImGui::PushID("TextRenderableComponent");
	const bool changed = GuiEditClass(trc, "TextRenderableComponent", [](auto& trc) {
		bool b = GuiEdit(trc.writer, "writer");
		b |= GuiEdit(trc.formatting, "formatting");
		b |= GuiEdit(trc.profile, "renderProfile");
		return b;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(SpriteRenderableComponent& spc)
{
	ImGui::PushID("SpriteRenderableComponent");
	const bool changed = GuiEditClass(spc, "SpriteRenderableComponent", [](auto& spc) {
		bool b = GuiEdit(spc.sprite, "sprite");
		b |= GuiEdit(spc.profile, "profile");
		return b;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(SpriteAnimationComponent& sac)
{
	ImGui::PushID("SpriteAnimationComponent");
	const bool changed = GuiEditClass(sac, "SpriteAnimationComponent", [](auto& sac) {
		bool b = GuiEdit(sac.spriteSeriesName, "spriteSeriesName");
		b |= GuiEdit(sac.index, "index");
		return b;
	});
	ImGui::PopID();
	return changed;
}

bool GuiEdit(TextRenderableGlyphCache& trgc)
{
	ImGui::PushID("TextRenderableGlyphCache");
	const bool changed = GuiEditClass(trgc, "TextRenderableGlyphCache", [](auto& trgc) {
		bool b = GuiEditContainer(trgc.cache, "cache");
		b |= GuiEdit(trgc.context, "context");
		return b;
	});
	ImGui::PopID();
	return changed;
}

// GUI EDIT PROPERTY //
PropertyEditState GuiEditProperty(GlyphTextWriter& wr)
{
	const auto& h = wr.resourceHandle;
	Property("resourceHandle", h);
	return Property("text", wr.text);
}

PropertyEditState GuiEditProperty(TextAlign& ta)
{
	static constexpr const char* kNames[] = {
		"Left",
		"Center",
		"Right"
	};
	int cur = ta == TextAlign::Left ? 0 :
		ta == TextAlign::Center ? 1 : 2;

	const bool changed = ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames));

	PropertyEditState state = PropertyEditState::None;

	if (ImGui::IsItemActivated())
	{
		state = PropertyEditState::Started;
	}

	if (changed)
	{
		switch (cur)
		{
		case 0: ta = TextAlign::Left; break;
		case 1: ta = TextAlign::Center; break;
		case 2: ta = TextAlign::Right; break;
		}

		state = PropertyEditState::Finished;
	}
	else if (state != PropertyEditState::Started && ImGui::IsItemActive())
	{
		state = PropertyEditState::Active;
	}

	return state;
}

PropertyEditState GuiEditProperty(Anchor& a)
{
	static constexpr const char* kNames[] = {
		"Left",
		"Right",
		"Top",
		"Bottom",
		"CenterX",
		"CenterY",
		"TopLeft",
		"TopRight",
		"BottomLeft",
		"BottomRight",
		"Center"
	};
	int cur = a == Anchor::Left ? 0 :
		a == Anchor::Right ? 1 :
		a == Anchor::Top ? 2 :
		a == Anchor::Bottom ? 3 :
		a == Anchor::CenterX ? 4 :
		a == Anchor::CenterY ? 5 :
		a == Anchor::TopLeft ? 6 :
		a == Anchor::TopRight ? 7 :
		a == Anchor::BottomLeft ? 8 :
		a == Anchor::BottomRight ? 9 : 10;

	const bool changed = ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames));

	PropertyEditState state = PropertyEditState::None;

	if (ImGui::IsItemActivated())
	{
		state = PropertyEditState::Started;
	}

	if (changed)
	{
		switch (cur)
		{
		case 0: a = Anchor::Left; break;
		case 1: a = Anchor::Right; break;
		case 2: a = Anchor::Top; break;
		case 3: a = Anchor::Bottom; break;
		case 4: a = Anchor::CenterX; break;
		case 5: a = Anchor::CenterY; break;
		case 6: a = Anchor::TopLeft; break;
		case 7: a = Anchor::TopRight; break;
		case 8: a = Anchor::BottomLeft; break;
		case 9: a = Anchor::BottomRight; break;
		case 10: a = Anchor::Center; break;
		}

		state = PropertyEditState::Finished;
	}
	else if (state != PropertyEditState::Started && ImGui::IsItemActive())
	{
		state = PropertyEditState::Active;
	}

	return state;
}

PropertyEditState GuiEditProperty(TextFormatting& f)
{
	auto state = Property("bounds", f.bounds);
	state |= Property("align", f.align);
	state |= Property("letterSpacing", f.letterSpacing);
	state |= Property("scaleToBounds", f.scaleToBounds);
	return state;
}

PropertyEditState GuiEditProperty(RGB& c)
{
	return GuiEditProperties<"r", "g", "b">(c.r, c.g, c.b);
}

PropertyEditState GuiEditProperty(SDL_BlendMode b)
{
	static constexpr const char* kNames[] = {
		"SDL_BLENDMODE_NONE",
		"SDL_BLENDMODE_BLEND",
		"SDL_BLENDMODE_ADD",
		"SDL_BLENDMODE_MOD",
		"SDL_BLENDMODE_MUL"
	};
	int cur = b == SDL_BLENDMODE_NONE ? 0 :
			  b == SDL_BLENDMODE_BLEND ? 1 :
			  b == SDL_BLENDMODE_ADD ? 2 :
			  b == SDL_BLENDMODE_MOD ? 3 : 4;

	const bool changed = ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames));

	PropertyEditState state = PropertyEditState::None;

	if (ImGui::IsItemActivated())
	{
		state = PropertyEditState::Started;
	}

	if (changed)
	{
		switch (cur)
		{
		case 0: b = SDL_BLENDMODE_NONE; break;
		case 1: b = SDL_BLENDMODE_BLEND; break;
		case 2: b = SDL_BLENDMODE_ADD; break;
		case 3: b = SDL_BLENDMODE_MOD; break;
		case 4: b = SDL_BLENDMODE_MUL; break;
		}

		state = PropertyEditState::Finished;
	}
	else if (state != PropertyEditState::Started && ImGui::IsItemActive())
	{
		state = PropertyEditState::Active;
	}

	return state;
}

PropertyEditState GuiEditProperty(SDL_RendererFlip& f)
{
	static constexpr const char* kNames[] = {
		"SDL_FLIP_NONE",
		"SDL_FLIP_HORIZONTAL",
		"SDL_FLIP_VERTICAL"
	};
	int cur = f == SDL_FLIP_NONE ? 0 :
			  f == SDL_FLIP_HORIZONTAL ? 1 : 2;

	const bool changed = ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames));

	PropertyEditState state = PropertyEditState::None;

	if (ImGui::IsItemActivated())
	{
		state = PropertyEditState::Started;
	}

	if (changed)
	{
		switch (cur)
		{
		case 0: f = SDL_FLIP_NONE; break;
		case 1: f = SDL_FLIP_HORIZONTAL; break;
		case 2: f = SDL_FLIP_VERTICAL; break;
		}

		state = PropertyEditState::Finished;
	}
	else if (state != PropertyEditState::Started && ImGui::IsItemActive())
	{
		state = PropertyEditState::Active;
	}

	return state;
}

PropertyEditState GuiEditProperty(TextureMods& tm)
{
	auto state = Property("color", tm.color);
	state |= Property("alpha", tm.alpha);
	state |= Property("blend", tm.blend);
	return state;
}

PropertyEditState GuiEditProperty(DebugDraw& dd)
{
	auto state = Property("on", dd.on);
	state |= Property("color", dd.color);
	return state;
}

PropertyEditState GuiEditProperty(DebugDrawSet& dds)
{
	auto state = PropertyGroup("boundingBox", [&dds] { return Property("", dds.boundingBox); });
	state |= PropertyGroup("collider", [&dds] { return Property("", dds.collider); });
	return state;
}

PropertyEditState GuiEditProperty(RenderProfile::Anchors& as)
{
	auto state = Property("scale", as.scale);
	state |= Property("rotation", as.rotation);
	return state;
}

PropertyEditState GuiEditProperty(RenderProfile& rp)
{
	auto state = Property("drawOrder", rp.drawOrder);
	state |= PropertyGroup("mods", [&rp] { return Property("", rp.mods); });
	state |= Property("flip", rp.flip);
	state |= Property("offset", rp.offset);
	state |= PropertyGroup("debugDraw", [&rp] { return Property("", rp.debugDraw); });
	state |= Property("isOverlay", rp.isOverlay);
	state |= Property("parallaxFactor", rp.parallaxFactor);
	state |= PropertyGroup("anchor", [&rp] { return Property("", rp.anchor); });
	return state;
}

PropertyEditState GuiEditProperty(AtlasPlot& ap)
{
	auto state = Property("rect", ap.rect);
	state |= Property("rotation", ap.rotation);
	return state;
}

PropertyEditState GuiEditProperty(Sprite& sp)
{
	const auto& h = sp.resourceHandle;
	Property("resourceHandle", h);

	return PropertyGroup("plot", [&sp] { return Property("", sp.plot); });
}

PropertyEditState GuiEditProperty(SpriteSeriesIndex& ssi)
{
	auto state = Property("current", ssi.current, DragArgs<int>{1.0f, 0, static_cast<int>(ssi.max) });
	const auto& max = ssi.max;
	Property("max", max);

	return state;
}

PropertyEditState GuiEditProperty(Glyph& g)
{
	auto state = Property("character", g.character);
	state |= PropertyGroup("plot", [&g] { return Property("", g.plot); });
	state |= Property("advance", g.advance);
	return state;
}

PropertyEditState GuiEditProperty(GlyphCacheData& gcd)
{
	auto state = PropertyGroup("glyph", [&gcd] { return Property("", gcd.glyph); });
	state |= Property("destRect", gcd.destRect);
	state |= Property("rotationCenter", gcd.rotationCenter);
	return state;
}

PropertyEditState GuiEditProperty(TextRenderableGlyphCache::CacheContext& ctx)
{
	auto state = PropertyGroup("transform", [&ctx] { return Property("", ctx.transform); });
	state |= PropertyGroup("formatting", [&ctx] { return Property("", ctx.formatting); });
	state |= Property("offset", ctx.offset);

	const auto& h = ctx.resourceHandle;
	Property("resourceHandle", h);

	state |= Property("textHash", ctx.textHash);
	return state;
}

PropertyEditState GuiEditProperty(TextRenderableComponent& trc)
{
	auto state = PropertyGroup("writer", [&trc] { return Property("", trc.writer); });
	state |= PropertyGroup("formatting", [&trc] { return Property("", trc.formatting); });
	state |= PropertyGroup("renderProfile", [&trc] { return Property("", trc.profile); });
	return state;
}

PropertyEditState GuiEditProperty(SpriteRenderableComponent& spc)
{
	auto state = PropertyGroup("sprite", [&spc] { return Property("", spc.sprite); });
	state |= PropertyGroup("profile", [&spc] { return Property("", spc.profile); });
	return state;
}

PropertyEditState GuiEditProperty(SpriteAnimationComponent& sac)
{
	auto state = Property("spriteSeriesName", sac.spriteSeriesName);
	state |= PropertyGroup("index", [&sac] { return Property("", sac.index); });
	return state;
}

PropertyEditState GuiEditProperty(TextRenderableGlyphCache& trgc)
{
	auto state = PropertyGroup("cache", [&trgc] { return Property("", trgc.cache); });
	state |= PropertyGroup("context", [&trgc] { return Property("", trgc.context); });
	return state;
}

} // ui

#endif