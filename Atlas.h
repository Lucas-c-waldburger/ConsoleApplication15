#pragma once
#include "SDLite.h"
#include "Core.h"

template <typename T> struct AtlasInfo;

template <typename Derived>
class Atlas
{
public:
	using AtlasInfo = AtlasInfo<Derived>;

	Atlas() { handle_ = Handle<Derived>::Create(); }
	~Atlas() { if (atlasTexture_) { SDL_DestroyTexture(atlasTexture_); } }

	SDL_Texture* GetAtlasTexture() { return atlasTexture_; }
	const AtlasInfo& GetAtlasInfo() const { return atlasInfo_; }
	const Handle<Derived>& GetHandle() const { return handle_; }
	bool IsLoaded() const { return atlasTexture_ != nullptr; }

protected:
	SDL_Texture* atlasTexture_ = nullptr;
	AtlasInfo atlasInfo_;
	Handle<Derived> handle_;
};

