#pragma once
#include "../sdl/SDLUtils.h"
#include "../core/Handle.h"
#include "../deps/RectangleBinPack/MaxRectsBinPack.h"
#include "ResourcePacket.h"
#include "../core/Result.h"

using UniqueTexturePtr = std::unique_ptr<SDL_Texture, 
	decltype([](SDL_Texture* t) { SDL_DestroyTexture(t); })>;

inline UniqueTexturePtr MakeUniqueTexturePtr(SDL_Renderer* renderer, SDL_PixelFormatEnum fmt,
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

	friend constexpr bool operator==(const AtlasPlot& lhs, const AtlasPlot& rhs)
	{
		return lhs.rect == rhs.rect && lhs.rotation == rhs.rotation;
	}
};

template <typename Derived>
class TextureAtlas
{
public:
	TextureAtlas() = default;
	~TextureAtlas() = default;

	TextureAtlas(const TextureAtlas&) = delete;
	TextureAtlas& operator=(const TextureAtlas&) = delete;

	TextureAtlas(TextureAtlas&& other) noexcept : atlasTexture_(std::move(other.atlasTexture_)),
		binPack_(std::move(other.binPack_)), handle_(other.handle_) {}

	TextureAtlas& operator=(TextureAtlas&& other) noexcept
	{
		if (this != &other)
		{
			atlasTexture_ = std::move(other.atlasTexture_);
			binPack_ = std::move(other.binPack_);
			handle_ = other.handle_;
		}
		return *this;
	}

	SDL_Texture* GetAtlasTexture() const { return atlasTexture_.get(); }
	const Handle<Derived>& GetHandle() const { return handle_; }

	template <typename LoadData>
	Result<Handle<Derived>> Load(SDL_Renderer* renderer, LoadData&& data);

	bool IsLoaded() const { return handle_.IsValid() && atlasTexture_; }

protected:
	UniqueTexturePtr atlasTexture_ = nullptr;
	rbp::MaxRectsBinPack binPack_;

private:
	Handle<Derived> handle_;
};


template <typename Derived>
template <typename LoadData>
inline Result<Handle<Derived>> TextureAtlas<Derived>::Load(SDL_Renderer* renderer, LoadData&& data)
{
	// clear our last state
	handle_ = {};
	atlasTexture_.reset();

	// do actual loading
	auto loadResult = static_cast<Derived*>(this)->LoadImpl(renderer, std::forward<LoadData>(data));

	// if success, generate a new handle and return it
	if (loadResult.Success())
	{
		handle_ = Handle<Derived>::Create();

		return handle_;
	}
	else
	{
		atlasTexture_.reset();

		return loadResult.GetError();
	}
}