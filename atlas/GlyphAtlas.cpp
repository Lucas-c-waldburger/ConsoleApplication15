#include "GlyphAtlas.h"
#include "PackingTools.h"
#include <SDL_ttf.h>
#include "NewGlyphAtlas.h"

namespace {

struct GlyphSurface
{
    Glyph data;
    SDL_Surface* surface = nullptr;
};

} // unnamed

Glyph GlyphAtlas::GetGlyph(char c) const
{
    Glyph invalid{ .character = kInvalidChar };

    if (!IsLoaded())
    {
        return invalid;
    }

    auto it = glyphMap_.find(c);

    return (it != glyphMap_.end()) ? it->second : invalid;
}

std::vector<Glyph> GlyphAtlas::GetGlyphsForString(std::string_view sv) const
{
    std::vector<Glyph> glyphs;
    glyphs.reserve(sv.size());

    for (char c : sv)
    {
        auto& glyph = glyphs.emplace_back(GetGlyph(c));
        if (glyph.character == kInvalidChar)
        {
            return {};
        }
    }

    return glyphs;
}

Result<Void> GlyphAtlas::LoadImpl(SDL_Renderer* renderer, FontResourcePacket&& packet)
{
    glyphMap_.clear();

    if (!packet.IsValid())
    {
        return MAKE_ERROR("Font resource packet was invalid");
    }

    fontResourcePacket_ = std::move(packet);

    auto metadataCopy = fontResourcePacket_.GetMetadata();
    const auto& filepaths = fontResourcePacket_.GetFilepaths();

    if (filepaths.size() != 1)
    {
        return MAKE_ERROR("Only one filepath should be set on a font resource packet");
    }

    TTF_Font* font = TTF_OpenFont(filepaths[0].c_str(), metadataCopy.fontSize);
    if (!font)
    {
        return MAKE_ERROR_FMT("Failed to load font: {}", TTF_GetError());
    }

    metadataCopy.fontHeight = TTF_FontHeight(font);
    fontResourcePacket_.SetMetadata(std::move(metadataCopy));
    const auto& metadata = fontResourcePacket_.GetMetadata();

    static_assert(kStartChar >= 0);
    static_assert(kEndChar > kStartChar);
    constexpr int numGlyphs = kEndChar - kStartChar;

    std::vector<GlyphSurface> glyphSurfaces{static_cast<size_t>(numGlyphs)};
    SDL_Surface* atlasSurface = nullptr;

    auto freeResources = [&, this]() {
        for (auto& glyphSurface : glyphSurfaces)
        {
            if (glyphSurface.surface)
            {
                SDL_FreeSurface(glyphSurface.surface);
            }
        }
        if (atlasSurface)
        {
            SDL_FreeSurface(atlasSurface);
        }
        if (font)
        {
            TTF_CloseFont(font);
        }

        glyphMap_.clear();
    };

    glyphMap_.reserve(static_cast<size_t>(numGlyphs));

    int totalArea = 0;
    int advance = 0;
    for (char c = kStartChar; c < kEndChar; c++)
    {
        auto& [glyph, surface] = glyphSurfaces[static_cast<size_t>(c - kStartChar)];

        glyph.character = c;

        if (TTF_GlyphMetrics32(font, c, nullptr, nullptr, nullptr, nullptr, &advance) == 0)
        {
            glyph.advance = advance;
        }
        else
        {
            freeResources();

            return MAKE_ERROR_FMT("Failed to get glyph metrics for {}: {}", c, TTF_GetError());
        }

        surface = TTF_RenderGlyph_Blended(font, c, metadata.fontColor);
        if (!surface)
        {
            freeResources();

            return MAKE_ERROR_FMT("Failed to make surface for char '{}': {}'", c, TTF_GetError());
        }

        if (surface->w <= 0 || surface->h <= 0)
        {
            freeResources();

            return MAKE_ERROR_FMT("Surface dimensions invalid: [{}, {}]", 
                                  surface->w, surface->h);
        }

        totalArea += surface->w * surface->h;
    }

    float idealSideLen = std::ceil(std::sqrt(static_cast<float>(totalArea)));
    int atlasSideLen = GetNextPowerOfTwo(static_cast<int>(idealSideLen));

    bool done = false;
    while (!done)
    {
        done = true;

        if (atlasSideLen > (1 << 30))
        {
            freeResources();

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

    atlasSurface = SDL_CreateRGBSurfaceWithFormat(
        0, atlasSideLen, atlasSideLen, 32, SDL_PIXELFORMAT_RGBA32);

    for (auto& [glyph, surface] : glyphSurfaces)
    {
        int blitted = SDL_BlitSurface(surface, nullptr, atlasSurface, &glyph.plot.rect);
        if (blitted < 0)
        {
            freeResources();

            return MAKE_ERROR_FMT("Failed to blit surface: {}", SDL_GetError());
        }

        SDL_FreeSurface(surface);

        glyphMap_[glyph.character] = std::move(glyph);
    }

    atlasTexture_ = MakeUniqueTexturePtrFromSurface(renderer, atlasSurface);

    SDL_FreeSurface(atlasSurface);

    if (!atlasTexture_)
    {
        glyphMap_.clear();

        return MAKE_ERROR_FMT("Failed to create atlas texture: {}", SDL_GetError());
    }

    SDL_SetTextureBlendMode(atlasTexture_.get(), SDL_BLENDMODE_BLEND);

    return Void{};
}