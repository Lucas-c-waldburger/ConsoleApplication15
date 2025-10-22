#pragma once
#include "Atlas.h"
#include "NewAtlas.h"
#include "TextureHandle.h"
#include "../core/Dictionary.h"

struct SpriteDescriptor
{
	std::string spriteName;
	std::string filepath;
};

struct Sprite
{
	Handle<NewTextureAtlas> sourceAtlas
};

class SpriteAtlas : public NewTextureAtlas
{
public:
	static constexpr size_t kDefaultAtlasSize = 1024;
	static constexpr size_t kMaxAtlasSize = 4096;
	
	SpriteAtlas() = default;
	~SpriteAtlas() = default;

	SpriteAtlas(const SpriteAtlas&) = delete;
	SpriteAtlas& operator=(const SpriteAtlas&) = delete;

	SpriteAtlas(SpriteAtlas&& other) noexcept : NewTextureAtlas(std::move(other)),
		spriteDescriptors_(std::move(other.spriteDescriptors_)),
		plots_(std::move(other.plots_)) 
	{}

	SpriteAtlas& operator=(SpriteAtlas&& other) noexcept
	{
		if (this != &other)
		{
			NewTextureAtlas::operator=(std::move(other));
			spriteDescriptors_ = std::move(other.spriteDescriptors_);
			plots_ = std::move(other.plots_);
		}
		return *this;
	}

	static Result<SpriteAtlas> Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize);

	Result<Texture> LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);
	Result<Texture> LoadSprites(SDL_Renderer* renderer, std::vector<SpriteDescriptor>&& descriptors);

	Result<std::vector<Texture>> LoadSpriteSeries(SDL_Renderer* renderer, 
		std::string_view seriesName, std::vector<SpriteDescriptor>&& descriptors);

	TextureHandle GetSpriteHandle(std::string_view spriteName) const;
	std::vector<TextureHandle> GetSpriteSeriesHandles(std::string_view spriteSeriesName) const;

	AtlasPlot GetAtlasPlot(const TextureHandle& handle) const;

	const SpriteDescriptor* GetSpriteDescriptor(const TextureHandle& handle) const;

private:
	Result<Texture> LoadImpl(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);

	std::vector<SpriteDescriptor> spriteDescriptors_;
	std::vector<AtlasPlot> plots_;
	UnorderedDictionary<Range<size_t>> spriteSeriesReferences_;
};