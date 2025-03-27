#include "GlyphAtlas.h"

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

std::vector<GlyphInfo> GlyphAtlas::GetGlyphsForString(std::string_view sv)
{
    if (!atlasTexture_) { return {}; }

    std::vector<GlyphInfo> glyphs;
    glyphs.reserve(sv.size());

    for (const auto c : sv)
    {
        glyphs.push_back(glyphMap_[c]);
    }

    return glyphs;
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