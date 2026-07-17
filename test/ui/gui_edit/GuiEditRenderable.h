#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../PropertyEditState.h"
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

PropertyEditState GuiEditProperty(GlyphTextWriter& wr);
PropertyEditState GuiEditProperty(TextAlign& ta);
PropertyEditState GuiEditProperty(Anchor& a);
PropertyEditState GuiEditProperty(TextFormatting& f);
PropertyEditState GuiEditProperty(RGB& c);
PropertyEditState GuiEditProperty(SDL_BlendMode b);
PropertyEditState GuiEditProperty(SDL_RendererFlip& f);
PropertyEditState GuiEditProperty(TextureMods& tm);
PropertyEditState GuiEditProperty(DebugDraw& dd);
PropertyEditState GuiEditProperty(DebugDrawSet& dds);
PropertyEditState GuiEditProperty(RenderProfile::Anchors& as);
PropertyEditState GuiEditProperty(RenderProfile& rp);
PropertyEditState GuiEditProperty(AtlasPlot& ap);
PropertyEditState GuiEditProperty(Sprite& sp);
PropertyEditState GuiEditProperty(SpriteSeriesIndex& ssi);
PropertyEditState GuiEditProperty(Glyph& g);
PropertyEditState GuiEditProperty(GlyphCacheData& gcd);
PropertyEditState GuiEditProperty(TextRenderableGlyphCache::CacheContext& ctx);

PropertyEditState GuiEditProperty(TextRenderableComponent& trc);
PropertyEditState GuiEditProperty(SpriteRenderableComponent& spc);
PropertyEditState GuiEditProperty(SpriteAnimationComponent& sac);
PropertyEditState GuiEditProperty(TextRenderableGlyphCache& trgc);

} // ui

#endif