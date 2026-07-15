#pragma once
#include <memory>
#include "../sdl/SDLUtils.h"
#include "TextureResourceHandle.h"
#include "../core/Signal.h"
#include "PackingTools.h"

using UniqueTexturePtr = std::unique_ptr<SDL_Texture,
	decltype([](SDL_Texture* t) { SDL_DestroyTexture(t); }) > ;

inline UniqueTexturePtr 
MakeUniqueTexturePtr(SDL_Renderer* renderer, SDL_PixelFormatEnum fmt,
					 SDL_TextureAccess access, int w, int h)
{
	return UniqueTexturePtr{ SDL_CreateTexture(renderer, fmt, access, w, h) };
}

inline UniqueTexturePtr MakeUniqueTexturePtrFromSurface(SDL_Renderer* renderer, SDL_Surface* surface)
{
	return UniqueTexturePtr{ SDL_CreateTextureFromSurface(renderer, surface) };
}

enum class TextureType
{
	Unknown = -1,
	Glyph,
	Sprite
};

struct AtlasPlot
{
	SDL_Rect rect = { 0, 0, 0, 0 };
	float rotation = 0.0f;

	friend constexpr bool operator==(const AtlasPlot& lhs, const AtlasPlot& rhs) noexcept
	{
		return lhs.rect == rhs.rect && lhs.rotation == rhs.rotation;
	}
};

static constexpr size_t kSizeMax = std::numeric_limits<size_t>::max();

enum class TextureGrowthPolicy
{
	FixedSize,
	FlexibleSize
};

class TextureAtlas
{
public:
	static constexpr size_t kDefaultAtlasSize = 1024;
	static constexpr size_t kMaxAtlasSize = 4096;

	TextureAtlas() = default;
	~TextureAtlas() = default;

	TextureAtlas(const TextureAtlas&) = delete;
	TextureAtlas& operator=(const TextureAtlas&) = delete;

	TextureAtlas(TextureAtlas&& other) noexcept :
		atlasTexture_(std::move(other.atlasTexture_)),
		binPack_(std::move(other.binPack_)),
		textureSize_(other.textureSize_),
		atlasId_(other.atlasId_)
	{}

	TextureAtlas& operator=(TextureAtlas&& other) noexcept
	{
		if (this != &other)
		{
			atlasTexture_ = std::move(other.atlasTexture_);
			binPack_ = std::move(other.binPack_);
			textureSize_ = other.textureSize_;
			atlasId_ = other.atlasId_;
		}
		return *this;
	}

	SDL_Texture* GetSourceTexture() const { return atlasTexture_.get(); }

	TextureAtlasID GetAtlasID() const { return atlasId_; }

	bool IsLoaded() const { return atlasId_ < atlasIdCounter && atlasTexture_; }

	size_t GetTextureSize() const noexcept { return textureSize_; }

protected:
	static constexpr TextureAtlasID GetNextAtlasID() { return atlasIdCounter++; }

	explicit TextureAtlas(TextureAtlasID id) : atlasId_(id) {}

	UniqueTexturePtr atlasTexture_;
	rbp::MaxRectsBinPack binPack_;
	size_t textureSize_ = 0;

private:
	static inline TextureAtlasID atlasIdCounter = 0;

	TextureAtlasID atlasId_ = kInvalidTextureAtlasID;
};