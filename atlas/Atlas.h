#pragma once
#include "../sdl/SDLite.h"
#include "../core/Handle.h"
#include "../deps/RectangleBinPack/MaxRectsBinPack.h"
#include "ResourcePacket.h"
#include "../core/Result.h"

template <typename T> struct AtlasInfo;

template <typename Derived>
class Atlas
{
public:
	using AtlasInfo = AtlasInfo<Derived>;

	Atlas() = default;
	Atlas(const Handle<Derived>& handle) : handle_(handle) {}
	~Atlas() { if (atlasTexture_) { SDL_DestroyTexture(atlasTexture_); } }

	SDL_Texture* GetAtlasTexture() const { return atlasTexture_; }
	const AtlasInfo& GetAtlasInfo() const { return atlasInfo_; }
	const Handle<Derived>& GetHandle() const { return handle_; }
	bool IsLoaded() const { return atlasTexture_ != nullptr; }

protected:
	SDL_Texture* atlasTexture_ = nullptr;
	AtlasInfo atlasInfo_;
	Handle<Derived> handle_;
};


struct AtlasPlot
{
	SDL_Rect rect = { 0, 0, 0, 0 };
	float rotation = 0.0f;
};

template <typename Derived>
class TextureAtlas
{
public:
	TextureAtlas() = default;
	~TextureAtlas() { if (atlasTexture_) { SDL_DestroyTexture(atlasTexture_); } }

	SDL_Texture* GetAtlasTexture() const { return atlasTexture_; }
	const Handle<Derived>& GetHandle() const { return handle_; }

	template <typename LoadData>
	Result<Handle<Derived>> Load(SDL_Renderer* renderer, LoadData&& data);

	bool IsLoaded() const { return handle_.IsValid() && atlasTexture_; }

protected:
	SDL_Texture* atlasTexture_ = nullptr;
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

	if (atlasTexture_)
	{
		SDL_DestroyTexture(atlasTexture_);
		atlasTexture_ = nullptr;
	}

	// do actual loading
	auto result = static_cast<Derived*>(this)->LoadImpl(renderer, std::forward<LoadData>(data));

	// if success, generate a new handle and return it
	if (result.Success())
	{
		handle_ = Handle<Derived>::Create();

		return handle_;
	}
	else
	{
		if (atlasTexture_)
		{
			SDL_DestroyTexture(atlasTexture_);
			atlasTexture_ = nullptr;
		}

		return result.GetError();
	}
}