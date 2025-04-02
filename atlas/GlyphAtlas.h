#pragma once
#include "Atlas.h"
#include "../core/Result.h"
#include <unordered_map>

static constexpr char kStartChar = 32;
static constexpr char kEndChar = 127;
static constexpr char kInvalidChar = kStartChar - 1;

struct GlyphInfo
{
    char character = kInvalidChar;
    SDL_Rect atlasRect = { 0, 0, 0, 0 };
    int advance = 0;
};

class GlyphAtlas;

template <>
struct AtlasInfo<GlyphAtlas>
{
    std::string fontPath;
    int fontSize = 0;
    SDL_Color fontColor = { 0, 0, 0, 255 };
    int fontHeight = 0;
};

class GlyphAtlas : public Atlas<GlyphAtlas>
{
public:
    GlyphAtlas() = default;
    GlyphAtlas(const Handle<GlyphAtlas>& handle) : Atlas(handle) {}

    bool Load(SDL_Renderer* renderer, AtlasInfo args);

    std::vector<GlyphInfo> GetGlyphsForString(std::string_view sv) const;

    GlyphInfo GetGlyph(char c) const;
    GlyphInfo operator[](char c) const;

private:
    int CalculateAtlasLengthAndPreFillMap(TTF_Font* font);

	std::unordered_map<char, GlyphInfo> glyphMap_;
};

