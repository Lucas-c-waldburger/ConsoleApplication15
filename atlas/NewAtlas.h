#pragma once
#include "TextureHandle.h"
#include "Atlas.h"
#include <variant>

static constexpr size_t kSizeMax = std::numeric_limits<size_t>::max();

class NewTextureAtlas
{
public:
	NewTextureAtlas() = default;
	~NewTextureAtlas() = default;

	NewTextureAtlas(const NewTextureAtlas&) = delete;
	NewTextureAtlas& operator=(const NewTextureAtlas&) = delete;

	NewTextureAtlas(NewTextureAtlas&& other) noexcept : 
		atlasTexture_(std::move(other.atlasTexture_)),
		binPack_(std::move(other.binPack_)), handle_(other.handle_) 
	{}

	NewTextureAtlas& operator=(NewTextureAtlas&& other) noexcept
	{
		if (this != &other)
		{
			atlasTexture_ = std::move(other.atlasTexture_);
			binPack_ = std::move(other.binPack_);
			handle_ = other.handle_;
		}
		return *this;
	}

	SDL_Texture* const GetSourceTexture() const { return atlasTexture_.get(); }

	const Handle<NewTextureAtlas>& GetAtlasHandle() const { return handle_; }

	bool IsLoaded() const { return handle_.IsValid() && atlasTexture_; }

protected:
	explicit NewTextureAtlas(Handle<NewTextureAtlas>&& handle) : 
		handle_(std::move(handle)) {}

	UniqueTexturePtr atlasTexture_;
	rbp::MaxRectsBinPack binPack_;

private:
	Handle<NewTextureAtlas> handle_;
};

