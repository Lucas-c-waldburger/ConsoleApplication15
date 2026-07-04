#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../../components/RenderableComponent.h"
#include "../../../components/SpriteAnimationsComponent.h"

namespace ui {

bool GuiEdit(GlyphTextWriter& wr, const char* label = "");
bool GuiEdit(TextAlign& ta, const char* label);
bool GuiEdit(Anchor& a, const char* label = "");
bool GuiEdit(TextFormatting& f, const char* label = "");
bool GuiEdit(RGB& c, const char* label = "");
bool GuiEdit(SDL_BlendMode b, const char* label = "");
bool GuiEdit(SDL_RendererFlip& f, const char* label = "");
bool GuiEdit(TextureMods& tm, const char* label = "");
bool GuiEdit(DebugDraw& dd, const char* label = "");
bool GuiEdit(DebugDrawSet& dds, const char* label = "");
bool GuiEdit(RenderProfile::Anchors& as, const char* label = "");
bool GuiEdit(RenderProfile& rp, const char* label = "");
bool GuiEdit(AtlasPlot& ap, const char* label = "");
bool GuiEdit(Sprite& sp, const char* label = "");
bool GuiEdit(SpriteSeriesIndex& ssi, const char* label = "");
bool GuiEdit(Glyph& g, const char* label = "");
bool GuiEdit(GlyphCacheData& gcd, const char* label = "");
bool GuiEdit(TextRenderableGlyphCache::CacheContext& ctx, const char* label = "");

bool GuiEdit(TextRenderableComponent& trc);
bool GuiEdit(SpriteRenderableComponent& spc);
bool GuiEdit(SpriteAnimationComponent& sac);
bool GuiEdit(TextRenderableGlyphCache& trgc);

bool GuiEditProperty(GlyphTextWriter& wr);
bool GuiEditProperty(TextAlign& ta);
bool GuiEditProperty(Anchor& a);
bool GuiEditProperty(TextFormatting& f);
bool GuiEditProperty(RGB& c);
bool GuiEditProperty(SDL_BlendMode b);
bool GuiEditProperty(SDL_RendererFlip& f);
bool GuiEditProperty(TextureMods& tm);
bool GuiEditProperty(DebugDraw& dd);
bool GuiEditProperty(DebugDrawSet& dds);
bool GuiEditProperty(RenderProfile::Anchors& as);
bool GuiEditProperty(RenderProfile& rp);
bool GuiEditProperty(AtlasPlot& ap);
bool GuiEditProperty(Sprite& sp);
bool GuiEditProperty(SpriteSeriesIndex& ssi);
bool GuiEditProperty(Glyph& g);
bool GuiEditProperty(GlyphCacheData& gcd);
bool GuiEditProperty(TextRenderableGlyphCache::CacheContext& ctx);

bool GuiEditProperty(TextRenderableComponent& trc);
bool GuiEditProperty(SpriteRenderableComponent& spc);
bool GuiEditProperty(SpriteAnimationComponent& sac);
bool GuiEditProperty(TextRenderableGlyphCache& trgc);

} // ui

#endif