#pragma once
#include "NewAtlas.h"
#include "../core/Result.h"
#include "../core/Dictionary.h"


struct SpriteDescriptor
{
	std::string spriteName;
	std::string filepath;
};

struct SpriteDescriptorPackage
{
	std::vector<SpriteDescriptor> descriptors;
	std::string seriesName;
};

struct Sprite
{
	Handle<TextureAtlas> sourceAtlas;
	AtlasPlot plot;
	size_t spriteIndex = kSizeMax;

	bool operator==(const Sprite&) const = default;
};

struct SpriteInfo
{
	std::string spriteName;
	std::string filepath;
	std::string seriesName;
	size_t seriesIndex = kSizeMax;

	bool operator==(const SpriteInfo&) const = default;
};

class SpriteAtlas : public TextureAtlas
{
public:
	static const SpriteInfo kInvalidSpriteInfo;

	static constexpr size_t kDefaultAtlasSize = 1024;
	static constexpr size_t kMaxAtlasSize = 4096;
	
	SpriteAtlas() = default;
	~SpriteAtlas() = default;

	SpriteAtlas(const SpriteAtlas&) = delete;
	SpriteAtlas& operator=(const SpriteAtlas&) = delete;

	SpriteAtlas(SpriteAtlas&& other) noexcept : TextureAtlas(std::move(other)),
		sprites_(std::move(other.sprites_)),
		spriteInfo_(std::move(other.spriteInfo_)),
		spriteSeriesRanges_(std::move(other.spriteSeriesRanges_))
	{}

	SpriteAtlas& operator=(SpriteAtlas&& other) noexcept
	{
		if (this != &other)
		{
			TextureAtlas::operator=(std::move(other));
			sprites_ = std::move(other.sprites_);
			spriteInfo_ = std::move(other.spriteInfo_);
			spriteSeriesRanges_ = std::move(other.spriteSeriesRanges_);
		}
		return *this;
	}

	static Result<SpriteAtlas> 
	Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize);

	Result<Sprite> LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);
	Result<std::vector<Sprite>> 
	LoadSprites(SDL_Renderer* renderer, SpriteDescriptorPackage&& package);

	Sprite GetSprite(std::string_view spriteName) const;
	std::vector<Sprite> GetSpriteSeries(std::string_view spriteSeriesName) const;

	const SpriteInfo& GetSpriteInfo(const Sprite& sprite) const;

	Result<Void> ValidateSprite(const Sprite& sprite) const;
	bool IsSpriteValid(const Sprite& sprite) const;

private:
	explicit SpriteAtlas(Handle<TextureAtlas>&& handle) : 
		TextureAtlas(std::move(handle)) {}

	Result<Sprite> LoadSpriteImpl(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);
	Result<std::vector<Sprite>>
	LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptorPackage&& package);

	std::vector<Sprite> sprites_;
	std::vector<SpriteInfo> spriteInfo_;
	UnorderedDictionary<Range<size_t>> spriteSeriesRanges_;
};