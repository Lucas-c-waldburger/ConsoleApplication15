#pragma once
#include "SpriteAtlas.h"
#include "../core/Signal.h"

class EventBus2;

struct NewSpriteInfo
{
	Handle<TextureAtlas> textureHandle;
	AtlasPlot plot;
	std::string spriteName;
	std::string filepath;
	std::string seriesName;
	size_t seriesIndex = kSizeMax;

	bool operator==(const NewSpriteInfo&) const = default;
};

using NewSpriteInfoSOA = StableSOA<
	NewSpriteInfo,
	&NewSpriteInfo::textureHandle,
	&NewSpriteInfo::plot,
	&NewSpriteInfo::spriteName,
	&NewSpriteInfo::filepath,
	&NewSpriteInfo::seriesName,
	&NewSpriteInfo::seriesIndex
>;

class NewSpriteAtlas : public TextureAtlas
{
public:
	NewSpriteAtlas() = default;
	~NewSpriteAtlas() = default;

	NewSpriteAtlas(const SpriteAtlas&) = delete;
	NewSpriteAtlas& operator=(const SpriteAtlas&) = delete;

	NewSpriteAtlas(NewSpriteAtlas&& other) noexcept;
	NewSpriteAtlas& operator=(NewSpriteAtlas&& other) noexcept;

	static Result<NewSpriteAtlas>
	Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize);

	Result<NewSpriteInfo> 
	LoadSprite(SDL_Renderer* renderer, const SpriteDescriptor& descriptor,
			   bool& atlasFull);

	Result<Void> RebuildSourceTexture(SDL_Renderer* renderer, 
									  const NewSpriteInfoSOA& spriteInfo,
									  size_t& runningIdxCounter);

private:
	explicit NewSpriteAtlas(Handle<TextureAtlas>&& handle) :
		TextureAtlas(std::move(handle)) {}
};

class SpriteAtlasCollection
{
public:
	using SpriteIndexMap = RapidHashUnorderedMap<size_t>;
	using SeriesRangeMap = RapidHashUnorderedMap<Range<size_t>>;

	SpriteAtlasCollection() = default;
	~SpriteAtlasCollection() = default;

	SpriteAtlasCollection(const SpriteAtlasCollection&) = delete;
	SpriteAtlasCollection& operator=(const SpriteAtlasCollection&) = delete;

	SpriteAtlasCollection(SpriteAtlasCollection&& other) noexcept = default;
	SpriteAtlasCollection& operator=(SpriteAtlasCollection&& other) noexcept = default;

	Result<Sprite> LoadSprite(SDL_Renderer* renderer, 
							  SpriteDescriptor&& descriptor);

	Result<std::vector<Sprite>> LoadSprites(SDL_Renderer* renderer,
											SpriteDescriptors&& descriptors);

	Sprite GetSprite(std::string_view spriteName);
	std::vector<Sprite> GetSpriteSeries(std::string_view seriesName);
	Sprite GetSpriteSeriesMember(std::string_view seriesName, size_t seriesIdx);
	size_t GetSpriteSeriesSize(std::string_view seriesName) const;

	bool HasSprite(std::string_view spriteName) const;
	bool HasSpriteSeries(std::string_view seriesName) const;

	bool IsSpriteValid(const Sprite& sprite) const;

	Result<Void> RebuildSourceTextures(SDL_Renderer* renderer);

	auto GetSpriteInfo(const Sprite& sprite) const;

	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto GetSpriteInfo(const Sprite& sprite) const;

	void ConnectTextureRebuildSignal(SDL_Renderer* renderer, EventBus2& bus);

private:
	Result<Sprite> LoadSpriteImpl(SDL_Renderer* renderer,
								  SpriteDescriptor&& descriptor);

	Result<std::vector<Sprite>> LoadSpritesImpl(SDL_Renderer* renderer,
												SpriteDescriptors&& descriptors);

	Sprite MakeSprite(size_t spriteIndex) const;

	std::vector<NewSpriteAtlas> spriteAtlases_;
	NewSpriteInfoSOA spriteInfo_;
	SpriteIndexMap spriteNameIndices_;
	SeriesRangeMap seriesNameRanges_;
	SignalToken rebuildTexturesSignalToken_;

	size_t textureSize_ = TextureAtlas::kDefaultAtlasSize;
};


template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
auto SpriteAtlasCollection::GetSpriteInfo(const Sprite& sprite) const
{
	using Ret = decltype(spriteInfo_.TryGetView<MemberPtrs...>(0));

	if (!IsSpriteValid(sprite))
	{
		return Ret{ std::nullopt };
	}

	const auto& cInfo = spriteInfo_;
	return cInfo.TryGetView<MemberPtrs...>(sprite.spriteIndex);
}