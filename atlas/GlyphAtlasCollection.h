#pragma once
#include "NewGlyphAtlas.h"
#include "../core/Hash.h"
#include "../core/StableSOA.h"
#include "TextureObserverSignal.h"

struct FontInfo
{
	TextureAtlasID atlasId;
	std::string fontName;
	std::string filepath;
	int fontSize = 0;
	int fontHeight = 0;

	bool operator==(const FontInfo & rhs) const = default;
};

//using FontInfo = FontDescriptor;

using FontInfoSOA = StableSOA<
	FontInfo,
	&FontInfo::atlasId,
	&FontInfo::fontName,
	&FontInfo::filepath,
	&FontInfo::fontSize,
	&FontInfo::fontHeight
>;

class FontAtlasTexture : public TextureAtlas
{
public:
	static constexpr char kStartChar = 32;
	static constexpr char kEndChar = 127;

	static inline constexpr Glyph kNewlineGlyph{ .character = '\n' };

	FontAtlasTexture() = default;
	~FontAtlasTexture() = default;

	FontAtlasTexture(const FontAtlasTexture&) = delete;
	FontAtlasTexture& operator=(const FontAtlasTexture&) = delete;

	FontAtlasTexture(FontAtlasTexture&& other) noexcept : 
		TextureAtlas(std::move(other)), glyphs_(std::move(other.glyphs_)),
		fontHeight_(other.fontHeight_)
	{}

	FontAtlasTexture& operator=(FontAtlasTexture&& other) noexcept
	{
		if (this != &other)
		{
			TextureAtlas::operator=(std::move(other));
			glyphs_ = std::move(other.glyphs_);
			fontHeight_ = other.fontHeight_;
		}
		return *this;
	}

	static Result<FontAtlasTexture> Create(SDL_Renderer* renderer, 
										FontDescriptor& descriptor);

	Glyph GetGlyph(char c) const;
	std::vector<Glyph> GetGlyphsForString(std::string_view text) const;

	Result<Void> RebuildSourceTexture(SDL_Renderer* renderer, std::string_view filepath,
									  int fontSize);

	int GetFontHeight() const { return fontHeight_; }

private:
	explicit FontAtlasTexture(TextureAtlasID atlasId) : TextureAtlas(atlasId) {}

	struct GlyphSurface
	{
		Glyph data;
		SDL_Surface* surface = nullptr;
	};

	Result<Void> LoadImpl(SDL_Renderer* renderer, FontDescriptor& descriptor);

	std::vector<Glyph> glyphs_;
	int fontHeight_ = 0;
};


class FontAtlas : public TextureCreationNotifier
{
public:
	static inline const FontAtlasTexture kInvalidGlyphAtlas{};

	using FontIndexMap = RapidHashUnorderedMap<size_t>;

	FontAtlas() = default;
	~FontAtlas() = default;

	FontAtlas(const FontAtlas&) = delete;
	FontAtlas& operator=(const FontAtlas&) = delete;

	FontAtlas(FontAtlas&&) noexcept = default;
	FontAtlas& operator=(FontAtlas&&) noexcept = default;

	Result<Handle<TextureResource>> LoadFont(SDL_Renderer* renderer, 
											 FontDescriptor&& fontDescriptor);
	 
	Result<Void> LoadFonts(SDL_Renderer* renderer,
						   FontDescriptors&& fontDescriptors);

	const FontAtlasTexture& GetFont(std::string_view fontName) const;
	const FontAtlasTexture& GetFont(const Handle<TextureResource>& handle) const;

	bool HasFont(std::string_view fontName) const; 
	bool HasFont(const Handle<TextureResource>& handle) const;

	GlyphTextWriter GetTextWriter(std::string_view fontName) const;

	bool IsTextWriterValid(const GlyphTextWriter& writer) const;

	Result<Void> RebuildSourceTextures(SDL_Renderer* renderer);

	size_t GetTextureCount() const;

	FontDescriptors ExportFontDescriptors() const;

	auto GetFontInfo(std::string_view fontName) const
	{
		using Ret = decltype(fontInfo_.TryGetView(0));

		auto it = fontNameIndices_.find(fontName);
		if (it == fontNameIndices_.end())
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = fontInfo_;
		return cInfo.TryGetView(it->second);
	}
	auto GetFontInfo(const Handle<TextureResource>& handle) const
	{
		using Ret = decltype(fontInfo_.TryGetView(0));

		assert(fontInfo_.Size() == fontAtlasTextures_.size());

		const size_t fontIdx = static_cast<size_t>(handle.GetResourceIndex());
		if (fontIdx >= fontInfo_.Size())
		{
			return Ret{ std::nullopt };
		}
		if (handle.GetAtlasID() != fontAtlasTextures_[fontIdx].GetAtlasID())
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = fontInfo_;
		return cInfo.TryGetView(fontIdx);
	}

	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto GetFontInfo(std::string_view fontName) const
	{
		using Ret = decltype(fontInfo_.TryGetView<MemberPtrs...>(0));

		auto it = fontNameIndices_.find(fontName);
		if (it == fontNameIndices_.end())
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = fontInfo_;
		return cInfo.TryGetView<MemberPtrs...>(it->second);
	}
	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto GetFontInfo(const Handle<TextureResource>& handle) const
	{
		using Ret = decltype(fontInfo_.TryGetView<MemberPtrs...>(0));

		assert(fontInfo_.Size() == fontAtlasTextures_.size());

		const size_t fontIdx = static_cast<size_t>(handle.GetResourceIndex());
		if (fontIdx >= fontInfo_.Size())
		{
			return Ret{ std::nullopt };
		}
		if (handle.GetAtlasID() != fontAtlasTextures_[fontIdx].GetAtlasID())
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = fontInfo_;
		return cInfo.TryGetView<MemberPtrs...>(fontIdx);
	}

	auto IterFontInfo() const
	{
		return fontInfo_.ForEach();
	}
	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto IterFontInfo() const
	{
		return fontInfo_.ForEach<MemberPtrs...>();
	}

private:
	std::vector<FontAtlasTexture> fontAtlasTextures_;
	FontInfoSOA fontInfo_;
	FontIndexMap fontNameIndices_;
};