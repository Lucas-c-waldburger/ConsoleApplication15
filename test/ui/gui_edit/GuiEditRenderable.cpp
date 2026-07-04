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
bool GuiEditProperty(GlyphTextWriter& wr)
{
	const auto& h = wr.resourceHandle;
	Property("resourceHandle", h);
	return Property("text", wr.text);
}

bool GuiEditProperty(TextAlign& ta)
{
	static constexpr const char* kNames[] = {
	"Left",
	"Center",
	"Right"
	};
	int cur = ta == TextAlign::Left ? 0 :
		ta == TextAlign::Center ? 1 : 2;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
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

bool GuiEditProperty(Anchor& a)
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

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
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

bool GuiEditProperty(TextFormatting& f)
{
	bool b = Property("bounds", f.bounds);
	b |= Property("align", f.align);
	b |= Property("letterSpacing", f.letterSpacing);
	b |= Property("scaleToBounds", f.scaleToBounds);
	return b;
}

bool GuiEditProperty(RGB& c)
{
	return GuiEditProperties<"r", "g", "b">(c.r, c.g, c.b);
}

bool GuiEditProperty(SDL_BlendMode b)
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
	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
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

bool GuiEditProperty(SDL_RendererFlip& f)
{
	static constexpr const char* kNames[] = {
		"SDL_FLIP_NONE",
		"SDL_FLIP_HORIZONTAL",
		"SDL_FLIP_VERTICAL"
	};
	int cur = f == SDL_FLIP_NONE ? 0 :
		f == SDL_FLIP_HORIZONTAL ? 1 : 2;
	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
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

bool GuiEditProperty(TextureMods& tm)
{
	bool b = Property("color", tm.color);
	b |= Property("alpha", tm.alpha);
	b |= Property("blend", tm.blend);
	return b;
}

bool GuiEditProperty(DebugDraw& dd)
{
	bool b = Property("on", dd.on);
	b |= Property("color", dd.color);
	return b;
}

bool GuiEditProperty(DebugDrawSet& dds)
{
	bool b = PropertyGroup("boundingBox", [&dds] { return Property("", dds.boundingBox); });
	b |= PropertyGroup("collider", [&dds] { return Property("", dds.collider); });
	return b;
}

bool GuiEditProperty(RenderProfile::Anchors& as)
{
	bool b = Property("scale", as.scale);
	b |= Property("rotation", as.rotation);
	return b;
}

bool GuiEditProperty(RenderProfile& rp)
{
	bool b = Property("drawOrder", rp.drawOrder);
	b |= PropertyGroup("mods", [&rp] { return Property("", rp.mods); });
	b |= Property("flip", rp.flip);
	b |= Property("offset", rp.offset);
	b |= PropertyGroup("debugDraw", [&rp] { return Property("", rp.debugDraw); });
	b |= Property("isOverlay", rp.isOverlay);
	b |= Property("parallaxFactor", rp.parallaxFactor);
	b |= PropertyGroup("anchor", [&rp] { return Property("", rp.anchor); });
	return b;
}

bool GuiEditProperty(AtlasPlot& ap)
{
	bool b = Property("rect", ap.rect);
	b |= Property("rotation", ap.rotation);
	return b;
}

bool GuiEditProperty(Sprite& sp)
{
	const auto& h = sp.resourceHandle;
	Property("resourceHandle", h);

	return PropertyGroup("plot", [&sp] { return Property("", sp.plot); });
}

bool GuiEditProperty(SpriteSeriesIndex& ssi)
{
	bool b = Property("current", ssi.current, DragArgs<int>{1.0f, 0, static_cast<int>(ssi.max) });
	const auto& max = ssi.max;
	Property("max", max);

	return b;

	//int cur = static_cast<int>(ssi.current);
	//bool changed = false;
	//if (ImGui::DragInt("##Value", &cur, 1.0f, 0, static_cast<int>(ssi.max)))
	//{
	//	ssi.current = static_cast<size_t>(cur);
	//	changed = true;
	//}
	//ImGui::LabelText("max", "%d", ssi.max);
	//return changed;
}

bool GuiEditProperty(Glyph& g)
{
	bool b = Property("character", g.character);
	b |= PropertyGroup("plot", [&g] { return Property("", g.plot); });
	b |= Property("advance", g.advance);
	return b;
}

bool GuiEditProperty(GlyphCacheData& gcd)
{
	bool b = PropertyGroup("glyph", [&gcd] { return Property("", gcd.glyph); });
	b |= Property("destRect", gcd.destRect);
	b |= Property("rotationCenter", gcd.rotationCenter);
	return b;
}

bool GuiEditProperty(TextRenderableGlyphCache::CacheContext& ctx)
{
	bool b = PropertyGroup("transform", [&ctx] { return Property("", ctx.transform); });
	b |= PropertyGroup("formatting", [&ctx] { return Property("", ctx.formatting); });
	b |= Property("offset", ctx.offset);

	const auto& h = ctx.resourceHandle;
	Property("resourceHandle", h);

	b |= Property("textHash", ctx.textHash);
	return b;
}

bool GuiEditProperty(TextRenderableComponent& trc)
{
	bool b = PropertyGroup("writer", [&trc] { return Property("", trc.writer); });
	b |= PropertyGroup("formatting", [&trc] { return Property("", trc.formatting); });
	b |= PropertyGroup("renderProfile", [&trc] { return Property("", trc.profile); });
	return b;
}

bool GuiEditProperty(SpriteRenderableComponent& spc)
{
	bool b = PropertyGroup("sprite", [&spc] { return Property("", spc.sprite); });
	b |= PropertyGroup("profile", [&spc] { return Property("", spc.profile); });
	return b;
}

bool GuiEditProperty(SpriteAnimationComponent& sac)
{
	bool b = Property("spriteSeriesName", sac.spriteSeriesName);
	b |= PropertyGroup("index", [&sac] { return Property("", sac.index); });
	return b;
}

bool GuiEditProperty(TextRenderableGlyphCache& trgc)
{
	bool b = PropertyGroup("cache", [&trgc] { return Property("", trgc.cache); });
	b |= PropertyGroup("context", [&trgc] { return Property("", trgc.context); });
	return b;
}

} // ui

#endif