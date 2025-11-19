#pragma once
#include "NewAtlas.h"
#include "../core/Result.h"
#include <ranges>

struct Glyph
{
	static constexpr char kInvalidChar = static_cast<char>(-1);

	char character = kInvalidChar;
	AtlasPlot plot;
	int advance = 0;
	bool operator==(const Glyph&) const = default;
};

struct FontDescriptor
{
    std::string fontName;
	std::string filepath;
    int fontSize = 0;
    int fontHeight = 0;
};

struct GlyphTextWriter
{
	Handle<TextureAtlas> sourceAtlas;
	std::string text;

	bool operator==(const GlyphTextWriter&) const = default;
};


class GlyphAtlas : public TextureAtlas
{
public:
	static constexpr char kStartChar = 32;
	static constexpr char kEndChar = 127;

	static inline constexpr Glyph kNewlineGlyph{ .character = '\n' };

	GlyphAtlas() = default;
	~GlyphAtlas() = default;

	GlyphAtlas(const GlyphAtlas&) = delete;
	GlyphAtlas& operator=(const GlyphAtlas&) = delete;

	GlyphAtlas(GlyphAtlas&& other) noexcept : TextureAtlas(std::move(other)),
		fontDescriptor_(std::move(other.fontDescriptor_)),
		glyphs_(std::move(other.glyphs_))
	{}

	GlyphAtlas& operator=(GlyphAtlas&& other) noexcept
	{
		if (this != &other)
		{
			TextureAtlas::operator=(std::move(other));
			fontDescriptor_ = std::move(other.fontDescriptor_);
			glyphs_ = std::move(other.glyphs_);
		}
		return *this;
	}

	static Result<GlyphAtlas> Create(SDL_Renderer* renderer, FontDescriptor&& descriptor);

	Glyph GetGlyph(char c) const;
	std::vector<Glyph> GetGlyphsForString(std::string_view text) const;

	GlyphTextWriter GetTextWriter() const { return { .sourceAtlas = GetHandle() }; }

	const FontDescriptor& GetFontDescriptor() const;

	bool IsTextWriterValid(const GlyphTextWriter& writer) const;

private:
	explicit GlyphAtlas(Handle<TextureAtlas>&& handle) :
		TextureAtlas(std::move(handle)) {
	}

	struct GlyphSurface
	{
		Glyph data;
		SDL_Surface* surface = nullptr;
	};

	Result<Void> LoadImpl(SDL_Renderer* renderer, FontDescriptor&& descriptor);

	FontDescriptor fontDescriptor_;
	std::vector<Glyph> glyphs_;
};

