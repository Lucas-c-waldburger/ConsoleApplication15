#pragma once
#include "../sdl/SDLite.h"
#include "../core/Handle.h"

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

