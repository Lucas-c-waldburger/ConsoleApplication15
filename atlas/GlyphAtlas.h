#pragma once
#include "Atlas.h"
#include "../core/Result.h"
#include "../core/commonObjects.h"
#include <unordered_map>

static constexpr char kStartChar = 32;
static constexpr char kEndChar = 127;
static constexpr char kInvalidChar = kStartChar - 1;

struct FontMetadata
{
    std::string fontName;
    int fontSize = 0;
    SDL_Color fontColor = { 0, 0, 0, 255 };
    int fontHeight = 0;
};

using FontResourcePacket = ResourcePacket<FontMetadata>;

struct Glyph
{
    char character = kInvalidChar;
    AtlasPlot plot;
    int advance = 0;
    bool operator==(const Glyph&) const = default;
};

class GlyphAtlas : public TextureAtlas<GlyphAtlas>
{
public:
    friend class TextureAtlas<GlyphAtlas>;

    GlyphAtlas() = default;
    ~GlyphAtlas() = default;

    GlyphAtlas(const GlyphAtlas&) = delete;
    GlyphAtlas& operator=(const GlyphAtlas&) = delete;

    GlyphAtlas(GlyphAtlas&& other) noexcept : TextureAtlas<GlyphAtlas>(std::move(other)),
        fontResourcePacket_(std::move(other.fontResourcePacket_)), glyphMap_(std::move(other.glyphMap_)) {}

    GlyphAtlas& operator=(GlyphAtlas&& other) noexcept
    {
        if (this != &other)
        {
            TextureAtlas<GlyphAtlas>::operator=(std::move(other));
            fontResourcePacket_ = std::move(other.fontResourcePacket_);
            glyphMap_ = std::move(other.glyphMap_);
        }
        return *this;
    }

    const FontMetadata& GetFontData() const { return fontResourcePacket_.GetMetadata(); }

    Glyph GetGlyph(char c) const;

    std::vector<Glyph> GetGlyphsForString(std::string_view sv) const;

private:
    Result<Void> LoadImpl(SDL_Renderer* renderer, FontResourcePacket&& packet);

    FontResourcePacket fontResourcePacket_;
    std::unordered_map<char, Glyph> glyphMap_;
};