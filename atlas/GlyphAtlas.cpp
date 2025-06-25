#include "GlyphAtlas.h"
#include "PackingTools.h"

namespace {

struct GlyphSurface
{
    GlyphData data;
    SDL_Surface* surface = nullptr;
};

} // unnamed

bool GlyphAtlas::Load(SDL_Renderer* renderer, AtlasInfo args)
{
    atlasInfo_ = std::move(args);

    TTF_Font* font = TTF_OpenFont(atlasInfo_.fontPath.c_str(), atlasInfo_.fontSize);
    if (!font)
    {
        std::cerr << "Failed to load font: " << TTF_GetError();

        return false;
    }

    atlasInfo_.fontHeight = TTF_FontHeight(font);

    const int sideLen = CalculateAtlasLengthAndPreFillMap(font);

    SDL_Surface* atlasSurface = SDL_CreateRGBSurfaceWithFormat(0, sideLen, sideLen, 32, SDL_PIXELFORMAT_RGBA32);
    if (!atlasSurface)
    {
        std::cerr << "Failed to create atlas surface: " << SDL_GetError();

        TTF_CloseFont(font);

        return false;
    }

    int rowHeight = 0;
    int x = 0, y = 0;

    for (char c = kStartChar; c < kEndChar; ++c)
    {
        SDL_Surface* glyphSurface = TTF_RenderGlyph_Blended(font, c, atlasInfo_.fontColor);
        if (!glyphSurface)
        {
            std::cerr << "Failed to make surface for char '" << c << "' : " << TTF_GetError();

            continue;
        }

        if (x + glyphSurface->w >= sideLen)
        {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }
        assert(y + glyphSurface->h < sideLen);

        glyphMap_[c].atlasRect = { x, y, glyphSurface->w, glyphSurface->h };

        SDL_BlitSurface(glyphSurface, nullptr, atlasSurface, &glyphMap_[c].atlasRect);

        x += glyphSurface->w;
        rowHeight = std::max(rowHeight, glyphSurface->h);

        SDL_FreeSurface(glyphSurface);
    }

    atlasTexture_ = SDL_CreateTextureFromSurface(renderer, atlasSurface);
    assert(atlasTexture_);

    SDL_SetTextureBlendMode(atlasTexture_, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(atlasSurface);
    TTF_CloseFont(font);

    return true;
}

std::vector<GlyphInfo> GlyphAtlas::GetGlyphsForString(std::string_view sv) const
{
    if (!atlasTexture_) { return {}; }

    std::vector<GlyphInfo> glyphs;
    glyphs.reserve(sv.size());

    for (const auto c : sv)
    {
        if (auto it = glyphMap_.find(c); it != glyphMap_.end())
        {
            glyphs.push_back(it->second);
        }
    }

    return glyphs;
}

GlyphInfo GlyphAtlas::GetGlyph(char c) const
{
    auto it = glyphMap_.find(c);

    return (it != glyphMap_.end()) ? it->second : GlyphInfo{};
}

GlyphInfo GlyphAtlas::operator[](char c) const
{
    auto it = glyphMap_.find(c);

    return (it != glyphMap_.end()) ? it->second : GlyphInfo{};
}

int GlyphAtlas::CalculateAtlasLengthAndPreFillMap(TTF_Font* font)
{
    assert(font);

    Dimensions<int> maxDims = { 0, TTF_FontHeight(font) };
    int adv = 0;
    for (char c = kStartChar; c < kEndChar; c++)
    {
        if (TTF_GlyphMetrics(font, c, nullptr, nullptr, nullptr, nullptr, &adv) == 0)
        {
            glyphMap_[c] = { .character = c, .advance = adv };

            maxDims.w = std::max(maxDims.w, adv);
        }
        else
        {
            std::cerr << "Failed to get glyph metrics for '" << c << "' : " << TTF_GetError();
        }
    }

    int totalArea = maxDims.w * maxDims.h * (kEndChar - kStartChar);
    int minSide = static_cast<int>(std::ceil(std::sqrt(totalArea)));

    return GetNextPowerOfTwo(minSide);
}

GlyphData GlyphAtlas2::GetGlyph(char c) const
{
    GlyphData invalid{ .character = kInvalidChar };

    if (!IsLoaded())
    {
        return invalid;
    }

    auto it = glyphMap_.find(c);

    return (it != glyphMap_.end()) ? it->second : invalid;
}

std::vector<GlyphData> GlyphAtlas2::GetGlyphsForString(std::string_view sv)
{
    std::vector<GlyphData> glyphs;
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

Result<Void> GlyphAtlas2::LoadImpl(SDL_Renderer* renderer, FontResourcePacket&& packet)
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

    auto freeResources = [this, &glyphSurfaces, &atlasSurface]() {
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

        glyphMap_.clear();
    };

    glyphMap_.reserve(static_cast<size_t>(numGlyphs));

    int totalArea = 0;
    int advance = 0;
    for (char c = kStartChar; c < kEndChar; c++)
    {
        auto& [glyph, surface] = glyphSurfaces[static_cast<size_t>(c)];

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

        glyph.plot.rect.w = surface->w;
        glyph.plot.rect.h = surface->h;

        if (glyph.plot.rect.w <= 0 || glyph.plot.rect.h <= 0)
        {
            freeResources();

            return MAKE_ERROR_FMT("Surface dimensions invalid: [{}, {}]", 
                glyph.plot.rect.w, glyph.plot.rect.h);
        }

        totalArea += surface->w * surface->h;
    }

    float idealSideLen = std::ceil(std::sqrt(static_cast<float>(totalArea)));
    int atlasSideLen = GetNextPowerOfTwo(static_cast<int>(idealSideLen));

    bool success = false;
    while (!success)
    {
        if (atlasSideLen > (1 << 30))
        {
            freeResources();

            return MAKE_ERROR("Not all rects could be packed in the maximum atlas size");
        }

        binPack_.Init(atlasSideLen, atlasSideLen);

        for (auto& [glyph, surface] : glyphSurfaces)
        {
            rbp::Rect packed = binPack_.Insert(glyph.plot.rect.w, glyph.plot.rect.h,
                                               rbp::MaxRectsBinPack::RectBestAreaFit);
            if (!WasRectPacked(packed))
            {
                atlasSideLen = GetNextPowerOfTwo(atlasSideLen);

                continue;
            }

            glyph.plot.rect.x = packed.x;
            glyph.plot.rect.y = packed.y;

            if (WasFlipped(packed, glyph.plot.rect))
            {
                glyph.plot.rotation = 90.0f;
            }

            glyphMap_[glyph.character] = std::move(glyph);
        }

        success = true;
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
    }

    atlasTexture_ = SDL_CreateTextureFromSurface(renderer, atlasSurface);

    SDL_FreeSurface(atlasSurface);

    if (!atlasTexture_)
    {
        glyphMap_.clear();

        return MAKE_ERROR_FMT("Failed to create atlas texture: {}", SDL_GetError());
    }

    SDL_SetTextureBlendMode(atlasTexture_, SDL_BLENDMODE_BLEND);

    return Void{};
}
