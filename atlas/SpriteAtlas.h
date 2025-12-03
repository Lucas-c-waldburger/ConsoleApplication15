#pragma once
#include "NewAtlas.h"
#include "../core/Result.h"
#include "../core/Dictionary.h"
#include "../core/StableSOA.h"
#include "../core/Hash.h"
#include "../deps/nlohmann/json_fwd.hpp"

struct SpriteDescriptor
{
	std::string spriteName;
	std::string filepath;

	bool operator==(const SpriteDescriptor&) const = default;
};

struct SpriteDescriptors
{
	std::vector<SpriteDescriptor> data;
	std::string seriesName;
};

using SpriteDescriptorPackage = std::vector<SpriteDescriptors>;

struct Sprite
{
	Handle<TextureAtlas> sourceAtlas;
	AtlasPlot plot;
	size_t spriteIndex = kSizeMax;

	bool operator==(const Sprite&) const = default;
};

struct SpriteInfo
{
	AtlasPlot plot;
	std::string spriteName;
	std::string filepath;
	std::string seriesName;
	size_t seriesIndex = kSizeMax;

	bool operator==(const SpriteInfo&) const = default;
};

using SpriteInfoSOA = StableSOA<
	SpriteInfo,
	&SpriteInfo::plot,
	&SpriteInfo::spriteName,
	&SpriteInfo::filepath,
	&SpriteInfo::seriesName,
	&SpriteInfo::seriesIndex
>;

class SpriteAtlas : public TextureAtlas
{
public:
	using SpriteIndexMap = RapidHashUnorderedMap<size_t>;
	using SeriesRangeMap = RapidHashUnorderedMap<Range<size_t>>;

	//friend void to_json(nlohmann::json& j, const SpriteAtlas& atlas);
	//friend void from_json(const nlohmann::json& j, SpriteAtlas& atlas);

	SpriteAtlas() = default;
	~SpriteAtlas() = default;

	SpriteAtlas(const SpriteAtlas&) = delete;
	SpriteAtlas& operator=(const SpriteAtlas&) = delete;

	SpriteAtlas(SpriteAtlas&& other) noexcept : TextureAtlas(std::move(other)),
		spriteInfo_(std::move(other.spriteInfo_)),
		spriteIndices_(std::move(other.spriteIndices_)),
		seriesRanges_(std::move(other.seriesRanges_))
	{}

	SpriteAtlas& operator=(SpriteAtlas&& other) noexcept
	{
		if (this != &other)
		{
			TextureAtlas::operator=(std::move(other));
			spriteInfo_ = std::move(other.spriteInfo_);
			spriteIndices_ = std::move(other.spriteIndices_);
			seriesRanges_ = std::move(other.seriesRanges_);
		}
		return *this;
	}

	static Result<SpriteAtlas> 
	Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize);

	static Result<SpriteAtlas>
	Create(SDL_Renderer* renderer, SpriteInfoSOA&& spriteInfos, size_t size);

	static Result<SpriteAtlas>
	Deserialize(SDL_Renderer* renderer, const nlohmann::json& j);

	SpriteDescriptorPackage ExportSpriteDescriptors() const;

	Result<Sprite> LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);

	Result<std::vector<Sprite>> LoadSprites(SDL_Renderer* renderer, 
											SpriteDescriptors&& descriptors);

	Sprite GetSprite(std::string_view spriteName) const;
	std::vector<Sprite> GetSpriteSeries(std::string_view spriteSeriesName) const;

	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto GetSpriteInfo(const Sprite& sprite) const
	{
		using Ret = decltype(spriteInfo_.TryGetView<MemberPtrs...>(0));

		auto validated = ValidateSprite(sprite);
		if (!validated.Success())
		{
			LOG_ERROR(validated.GetError());

			return Ret{ std::nullopt };
		}

		return spriteInfo_.TryGetView<MemberPtrs...>(sprite.spriteIndex);
	}

	auto GetSpriteInfo(const Sprite& sprite) const
	{
		using Ret = decltype(spriteInfo_.TryGetView(0));

		auto validated = ValidateSprite(sprite);
		if (!validated.Success())
		{
			LOG_ERROR(validated.GetError());

			return Ret{ std::nullopt };
		}

		return spriteInfo_.TryGetView(sprite.spriteIndex);
	}

	//SpriteInfo GetSpriteInfo(const Sprite& sprite) const;

	Result<Void> ValidateSprite(const Sprite& sprite) const;
	bool IsSpriteValid(const Sprite& sprite) const;

	/*bool CanFitSprite(SDL_Renderer* renderer, const SpriteDescriptor& descriptor) const;*/

private:
	explicit SpriteAtlas(Handle<TextureAtlas>&& handle) : 
		TextureAtlas(std::move(handle)) {}

	Sprite MakeSprite(size_t spriteIndex) const;

	//Result<Sprite> LoadSpriteImpl(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);
	//Result<std::vector<Sprite>>
	//LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptors&& package);

	Result<Sprite> LoadSpriteImpl(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);

	Result<std::vector<Sprite>>
	LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptors&& descri);

	SpriteInfoSOA spriteInfo_;
	SpriteIndexMap spriteIndices_;
	SeriesRangeMap seriesRanges_;
};






//template <auto...MemberPtrs>
//auto BaseSpriteAtlas::ViewSpriteInfo(const Sprite& sprite) const
//{
//	auto validated = ValidateSprite(sprite);
//	if (!validated.Success())
//	{
//		LOG_ERROR(validated.GetError());
//
//		return std::nullopt;
//	}
//
//	return spriteInfo_.TryGetView<MemberPtrs...>(sprite.spriteIndex);
//}