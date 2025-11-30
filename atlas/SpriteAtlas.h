#pragma once
#include "NewAtlas.h"
#include "../core/Result.h"
#include "../core/Dictionary.h"
#include "../core/StableSOA.h"

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

static constexpr size_t kNoSpriteSeries = std::numeric_limits<size_t>::max();

struct Sprite
{
	Handle<TextureAtlas> sourceAtlas;
	AtlasPlot plot;
	size_t spriteIndex = kNoSpriteSeries;

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


//class BaseSpriteAtlas : public TextureAtlas
//{
//public:
//	static constexpr size_t kDefaultAtlasSize = 1024;
//	static constexpr size_t kMaxAtlasSize = 4096;
//
//	using SeriesIndexRangeMap = UnorderedDictionary<Range<size_t>>;
//
//	template <typename BasicJson>
//	friend void to_json(BasicJson&, const BaseSpriteAtlas&);
//
//	BaseSpriteAtlas() = default;
//	~BaseSpriteAtlas() = default;
//
//	BaseSpriteAtlas(const BaseSpriteAtlas&) = delete;
//	BaseSpriteAtlas& operator=(const BaseSpriteAtlas&) = delete;
//
//	BaseSpriteAtlas(BaseSpriteAtlas&& other) noexcept : TextureAtlas(std::move(other)),
//		spriteInfo_(std::move(other.spriteInfo_)),
//		spriteSeriesRanges_(std::move(other.spriteSeriesRanges_))
//	{}
//
//	BaseSpriteAtlas& operator=(BaseSpriteAtlas&& other) noexcept
//	{
//		if (this != &other)
//		{
//			TextureAtlas::operator=(std::move(other));
//			spriteInfo_ = std::move(other.spriteInfo_);
//			spriteSeriesRanges_ = std::move(other.spriteSeriesRanges_);
//		}
//		return *this;
//	}
//
//	Sprite GetSprite(std::string_view spriteName) const;
//	std::vector<Sprite> GetSpriteSeries(std::string_view spriteSeriesName) const;
//
//	template <auto...MemberPtrs>
//	auto ViewSpriteInfo(const Sprite& sprite) const;
//
//	SpriteInfo GetSpriteInfo(const Sprite& sprite) const;
//
//	Result<Void> ValidateSprite(const Sprite& sprite) const;
//	bool IsSpriteValid(const Sprite& sprite) const;
//
//protected:
//	explicit BaseSpriteAtlas(Handle<TextureAtlas>&& handle) :
//		TextureAtlas(std::move(handle)) 
//	{}
//
//	Sprite MakeSprite(size_t spriteIndex) const;
//
//	SpriteInfoSOA spriteInfo_;
//	SeriesIndexRangeMap spriteSeriesRanges_;
//};
//
//class DynamicSpriteAtlas : public BaseSpriteAtlas
//{
//public:
//	DynamicSpriteAtlas() = default;
//	~DynamicSpriteAtlas() = default;
//
//	DynamicSpriteAtlas(const DynamicSpriteAtlas&) = delete;
//	DynamicSpriteAtlas& operator=(const DynamicSpriteAtlas&) = delete;
//
//	DynamicSpriteAtlas(DynamicSpriteAtlas&& other) noexcept : 
//		BaseSpriteAtlas(std::move(other))
//	{}
//
//	DynamicSpriteAtlas& operator=(DynamicSpriteAtlas&& other) noexcept
//	{
//		if (this != &other)
//		{
//			BaseSpriteAtlas::operator=(std::move(other));
//		}
//		return *this;
//	}
//
//	static Result<DynamicSpriteAtlas>
//	Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize);
//
//	Result<Sprite> LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);
//
//	Result<std::vector<Sprite>>
//	LoadSprites(SDL_Renderer* renderer, SpriteDescriptorPackage&& package);
//
//private:
//	explicit DynamicSpriteAtlas(Handle<TextureAtlas>&& handle) :
//		BaseSpriteAtlas(std::move(handle)) 
//	{}
//
//	Result<Sprite>
//	LoadSpriteImpl(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);
//
//	Result<std::vector<Sprite>>
//	LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptorPackage&& package);
//};
//
//class FixedSpriteAtlas : public BaseSpriteAtlas
//{
//public:
//	FixedSpriteAtlas() = default;
//	~FixedSpriteAtlas() = default;
//
//	FixedSpriteAtlas(const FixedSpriteAtlas&) = delete;
//	FixedSpriteAtlas& operator=(const FixedSpriteAtlas&) = delete;
//
//	FixedSpriteAtlas(FixedSpriteAtlas&& other) noexcept :
//		BaseSpriteAtlas(std::move(other))
//	{}
//
//	FixedSpriteAtlas& operator=(FixedSpriteAtlas&& other) noexcept
//	{
//		if (this != &other)
//		{
//			BaseSpriteAtlas::operator=(std::move(other));
//		}
//		return *this;
//	}
//
//	static Result<FixedSpriteAtlas>
//	Create(SDL_Renderer* renderer, SpriteInfoSOA&& spriteInfo, 
//		   UnorderedDictionary<Range<size_t>>&& seriesRanges, size_t origAtlasSize);
//
//private:
//	explicit FixedSpriteAtlas(Handle<TextureAtlas>&& handle) :
//		BaseSpriteAtlas(std::move(handle))
//	{}
//};

//struct SpriteMaps
//{
//	std::unordered_map<std::string_view, size_t> spriteIndices;
//	std::unordered_map<std::string_view, Range<size_t>> seriesRanges;
//	SpriteInfoSOA spriteInfo;
//
//	Sprite GetSprite(std::string_view spriteName) const
//	{
//		auto it = spriteIndices.find(spriteName);
//
//		return (it != spriteIndices.end()) ? it->second : kSizeMax;
//	}
//
//	std::vector<Sprite> GetSpriteSeries(std::string_view seriesName) const
//	{
//		auto it = seriesRanges.find(seriesName);
//
//		return (it != seriesRanges.end()) ? it->second : Range<size_t>{ kSizeMax, kSizeMax };
//	}
//};


class SpriteAtlas : public TextureAtlas
{
public:
	using SpriteIndexMap = std::unordered_map<std::string_view, size_t>;
	using SeriesRangeMap = std::unordered_map<std::string_view, Range<size_t>>;

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

	Result<Sprite> LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);

	Result<std::vector<Sprite>> LoadSprites(SDL_Renderer* renderer, 
											SpriteDescriptors&& descriptors);

	Sprite GetSprite(std::string_view spriteName) const;
	std::vector<Sprite> GetSpriteSeries(std::string_view spriteSeriesName) const;

	template <auto...MemberPtrs>
	auto GetSpriteInfo(const Sprite& sprite) const;

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

template <auto...MemberPtrs>
auto SpriteAtlas::GetSpriteInfo(const Sprite& sprite) const
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