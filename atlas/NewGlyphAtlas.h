#pragma once
#include "NewAtlas.h"
#include "GlyphAtlas.h"


struct FontDescriptor
{
    std::string fontName;
	std::string filepath;
    int fontSize = 0;
    int fontHeight = 0;
};

struct GlyphText
{
	std::vector<Glyph> text;
	GlyphText& operator=(std::string_view sv)
	{
		text.resize(sv.size());
		for (size_t i = 0; i < text.size(); i++)
		{
			text[i].character = sv[i];
		}
	}
};

class NewGlyphAtlas : public NewTextureAtlas
{
public:
	static constexpr char kStartChar = 32;
	static constexpr char kEndChar = 127;
	static constexpr char kInvalidChar = kStartChar - 1;

	NewGlyphAtlas() = default;
	~NewGlyphAtlas() = default;

	NewGlyphAtlas(const NewGlyphAtlas&) = delete;
	NewGlyphAtlas& operator=(const NewGlyphAtlas&) = delete;

	NewGlyphAtlas(NewGlyphAtlas&& other) noexcept : NewTextureAtlas(std::move(other)),
		fontDescriptor_(std::move(other.fontDescriptor_)),
		glyphs_(std::move(other.glyphs_))
	{}

	NewGlyphAtlas& operator=(NewGlyphAtlas&& other) noexcept
	{
		if (this != &other)
		{
			NewTextureAtlas::operator=(std::move(other));
			fontDescriptor_ = std::move(other.fontDescriptor_);
			glyphs_ = std::move(other.glyphs_);
		}
		return *this;
	}

	static Result<NewGlyphAtlas> Create(SDL_Renderer* renderer, FontDescriptor&& descriptor);

	Glyph GetGlyph(char c) const;
	std::vector<Glyph> GetGlyphsForString(std::string_view text) const;

	const FontDescriptor& GetFontDescriptor() const;

	Result<Void> ValidateGlyph(const Glyph& glyph) const;

private:
	explicit NewGlyphAtlas(Handle<NewTextureAtlas>&& handle) :
		NewTextureAtlas(std::move(handle)) {
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

