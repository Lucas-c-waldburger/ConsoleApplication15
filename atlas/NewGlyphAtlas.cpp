#include "NewGlyphAtlas.h"
#include <SDL_ttf.h>
#include "../core/ScopedInvoker.h"
#include "PackingTools.h"

namespace {

static constexpr size_t GetPlotIndexForChar(char c)
{
    if (c < kStartChar || c > kEndChar)
    {
        return std::numeric_limits<size_t>::max();
    }

    return static_cast<size_t>(c - kStartChar);
}

} // unnamed namespace 

Result<Void> NewGlyphAtlas::LoadImpl(SDL_Renderer* renderer, FontDescriptor&& descriptor)
{
    fontDescriptor_ = std::move(fontDescriptor_);

    TTF_Font* font = TTF_OpenFont(fontDescriptor_.filepath.c_str(), fontDescriptor_.fontSize);
    if (!font)
    {
        return MAKE_ERROR_FMT("Failed to load font: {}", TTF_GetError());
    }

    fontDescriptor_.fontHeight = TTF_FontHeight(font);

    static constexpr size_t numGlyphs = static_cast<size_t>(kEndChar - kStartChar);
    static constexpr SDL_Color white = { 255, 255, 255, 255 };

    std::vector<GlyphSurface> glyphSurfaces{ numGlyphs };
    SDL_Surface* atlasSurface = nullptr;

    ScopedInvoker freeResources{ [&] {
        if (font) { TTF_CloseFont(font); }
        for (auto& [_, surf] : glyphSurfaces) { SDL_FreeSurface(surf); }
        SDL_FreeSurface(atlasSurface);
    }};

    // fill glyph metrics and calculate total area for atlas
    int totalArea = 0;
    for (char c = kStartChar; c < kEndChar; c++)
    {
        auto& [glyph, surface] = glyphSurfaces[GetPlotIndexForChar(c)];

        glyph.character = c;

        if (TTF_GlyphMetrics32(font, c, nullptr, nullptr, nullptr, nullptr, &glyph.advance) != 0)
        {
            return MAKE_ERROR_FMT("Failed to get glyph metrics for char '{}': {}", 
                c, TTF_GetError());
        }

        surface = TTF_RenderGlyph_Blended(font, c, white);
        if (!surface)
        {
            return MAKE_ERROR_FMT("Failed to make surface for char '{}': {}'", 
                c, TTF_GetError());
        }

        if (surface->w <= 0 || surface->h <= 0)
        {
            return MAKE_ERROR_FMT("Surface dimensions invalid: ({}, {})",
                surface->w, surface->h);
        }

        totalArea += surface->w * surface->h;
    }

    float idealSideLen = std::ceil(std::sqrt(static_cast<float>(totalArea)));
    int atlasSideLen = GetNextPowerOfTwo(static_cast<int>(idealSideLen));

    // assign each glyph to a plot on the atlas texture
    bool done = false;
    while (!done)
    {
        done = true;

        if (atlasSideLen > (1 << 30))
        {
            return MAKE_ERROR("Not all rects could be packed in the maximum atlas size");
        }

        binPack_.Init(atlasSideLen, atlasSideLen, false);

        for (auto& [glyph, surface] : glyphSurfaces)
        {
            rbp::Rect packed = binPack_.Insert(surface->w, surface->h,
                                               rbp::MaxRectsBinPack::RectBestAreaFit);
            if (!WasRectPacked(packed))
            {
                atlasSideLen *= 2;

                done = false;
                 
                break;
            }

            glyph.plot.rect = RbpToSDLRect(packed);
        }
    }

    // make atlas texture, blit the glyph surfaces on it
    atlasSurface = SDL_CreateRGBSurfaceWithFormat(0, atlasSideLen, atlasSideLen, 32, 
                                                  SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(atlasSurface, nullptr, SDL_MapRGBA(atlasSurface->format, 0, 0, 0, 0));

    for (auto& [glyph, surface] : glyphSurfaces)
    {
        if (SDL_BlitSurface(surface, nullptr, atlasSurface, &glyph.plot.rect) < 0) 
        {
            return MAKE_ERROR_FMT("Failed to blit surface: {}", SDL_GetError());
        }

        SDL_FreeSurface(surface);

        glyphs_.emplace_back(std::move(glyph));
    }

    atlasTexture_ = MakeUniqueTexturePtrFromSurface(renderer, atlasSurface);
    if (!atlasTexture_)
    {
        return MAKE_ERROR_FMT("Failed to create atlas texture: {}", SDL_GetError());
    }

    SDL_FreeSurface(atlasSurface);

    SDL_SetTextureBlendMode(atlasTexture_.get(), SDL_BLENDMODE_BLEND);

    freeResources.Release();

    return Void{};
}

Result<NewGlyphAtlas> NewGlyphAtlas::Create(SDL_Renderer* renderer, FontDescriptor&& descriptor)
{
    NewGlyphAtlas glyphAtlas{};

    TRY(glyphAtlas.LoadImpl(renderer, std::move(descriptor)));

    glyphAtlas.SetHandle(Handle<NewTextureAtlas>::Create(TextureType::Glyph));

    return glyphAtlas;
}

const FontDescriptor& NewGlyphAtlas::GetFontDescriptor() const
{
    return fontDescriptor_;
}

Glyph NewGlyphAtlas::GetGlyph(char c) const
{
    const size_t idx = GetPlotIndexForChar(c);

    return (idx < glyphs_.size()) ? glyphs_[idx] : Glyph{};
}