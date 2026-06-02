#pragma once
#include "SpriteAtlas.h"
#include "TextureObserverSignal.h"

class EventBus;

struct SpriteInfo
{
	TextureAtlasID atlasId;
	AtlasPlot plot;
	std::string spriteName;
	std::string filepath;
	std::string seriesName;
	size_t seriesIndex = std::numeric_limits<size_t>::max();

	bool operator==(const SpriteInfo&) const = default;
};

using SpriteInfoSOA = StableSOA<
	SpriteInfo,
	&SpriteInfo::atlasId,
	&SpriteInfo::plot,
	&SpriteInfo::spriteName,
	&SpriteInfo::filepath,
	&SpriteInfo::seriesName,
	&SpriteInfo::seriesIndex
>;

class SpriteAtlasTexture : public TextureAtlas
{
public:
	struct SpriteLoadOutcome
	{
		enum : uint8_t
		{
			Success,
			AtlasFull,
			SpriteTooLarge
		};

		uint8_t code = Success;	
		SpriteInfo spriteInfo{};
	};

	SpriteAtlasTexture() = default;
	~SpriteAtlasTexture() = default;

	SpriteAtlasTexture(const SpriteAtlasTexture&) = delete;
	SpriteAtlasTexture& operator=(const SpriteAtlasTexture&) = delete;

	SpriteAtlasTexture(SpriteAtlasTexture&& other) noexcept;
	SpriteAtlasTexture& operator=(SpriteAtlasTexture&& other) noexcept;

	static Result<SpriteAtlasTexture>
	Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize);

	Result<SpriteLoadOutcome> 
	LoadSprite(SDL_Renderer* renderer, const SpriteDescriptor& descriptor);

	Result<Void> RebuildSourceTexture(SDL_Renderer* renderer, 
									  const SpriteInfoSOA& spriteInfo,
									  size_t& runningIdxCounter);

private:
	explicit SpriteAtlasTexture(TextureAtlasID atlasId) : TextureAtlas(atlasId) {}
};

class SpriteAtlas : public TextureCreationNotifier
{
public:
	using SpriteIndexMap = RapidHashUnorderedMap<size_t>;
	using SeriesRangeMap = RapidHashUnorderedMap<Range<size_t>>;

	SpriteAtlas() = default;
	SpriteAtlas(size_t txSize) : textureSize_(txSize) {}
	SpriteAtlas(size_t txSize, TextureGrowthPolicy policy) : 
		textureSize_(txSize), growthPolicy_(policy) {}
	~SpriteAtlas() = default;

	SpriteAtlas(const SpriteAtlas&) = delete;
	SpriteAtlas& operator=(const SpriteAtlas&) = delete;

	SpriteAtlas(SpriteAtlas&& other) noexcept = default;
	SpriteAtlas& operator=(SpriteAtlas&& other) noexcept = default;

	Result<Sprite> LoadSprite(SDL_Renderer* renderer, 
							  SpriteDescriptor&& descriptor);

	Result<std::vector<Sprite>> LoadSprites(SDL_Renderer* renderer,
											SpriteDescriptors&& descriptors);

	Sprite GetSprite(std::string_view spriteName) const;
	Sprite GetSprite(const Handle<TextureResource>& handle) const;
	std::vector<Sprite> GetSpriteSeries(std::string_view seriesName) const;
	Sprite GetSpriteSeriesMember(std::string_view seriesName, size_t seriesIdx) const;
	size_t GetSpriteSeriesSize(std::string_view seriesName) const;

	bool HasSprite(std::string_view spriteName) const;
	bool HasSpriteSeries(std::string_view seriesName) const;

	bool IsSpriteValid(const Sprite& sprite) const;

	Result<Void> RebuildSourceTextures(SDL_Renderer* renderer);

	auto GetSpriteInfo(const Handle<TextureResource>& handle) const
	{
		using Ret = decltype(spriteInfo_.TryGetView(0));

		const size_t spriteIdx = static_cast<size_t>(handle.GetResourceIndex());
		if (spriteIdx >= spriteInfo_.Size())
		{
			return Ret{ std::nullopt };
		}
		if (handle.GetAtlasID() != 
			spriteInfo_.GetView<&SpriteInfo::atlasId>(spriteIdx))
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = spriteInfo_;
		return cInfo.TryGetView(spriteIdx);
	}
	auto GetSpriteInfo(const Sprite& sprite) const
	{
		return GetSpriteInfo(sprite.resourceHandle);
	}

	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto GetSpriteInfo(const Handle<TextureResource>& handle) const
	{
		using Ret = decltype(spriteInfo_.TryGetView<MemberPtrs...>(0));

		const size_t spriteIdx = static_cast<size_t>(handle.GetResourceIndex());
		if (spriteIdx >= spriteInfo_.Size())
		{
			return Ret{ std::nullopt };
		}
		if (handle.GetAtlasID() !=
			spriteInfo_.GetView<&SpriteInfo::atlasId>(spriteIdx))
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = spriteInfo_;
		return cInfo.TryGetView<MemberPtrs...>(spriteIdx);
	}
	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto GetSpriteInfo(const Sprite& sprite) const
	{
		return GetSpriteInfo<MemberPtrs...>(sprite.resourceHandle);
	}

	auto IterSpriteInfo() const
	{
		return spriteInfo_.ForEach();
	}
	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto IterSpriteInfo() const
	{
		return spriteInfo_.ForEach<MemberPtrs...>();
	}

	size_t GetTextureCount() const;

	size_t GetTextureSize() const;
	void SetTextureSize(size_t newSize);

	TextureGrowthPolicy GetTextureGrowthPolicy() const;
	void SetTextureGrowthPolicy(TextureGrowthPolicy newPolicy);

	SpriteDescriptorPackage ExportSpriteDescriptors() const;

private:
	Result<Sprite> LoadSpriteImpl(SDL_Renderer* renderer,
								  SpriteDescriptor&& descriptor);

	Result<std::vector<Sprite>> LoadSpritesImpl(SDL_Renderer* renderer,
												SpriteDescriptors&& descriptors);

	Sprite MakeSprite(size_t spriteIndex) const;

	std::vector<SpriteAtlasTexture> spriteAtlasTextures_;
	SpriteInfoSOA spriteInfo_;
	SpriteIndexMap spriteNameIndices_;
	SeriesRangeMap seriesNameRanges_;
	SignalToken rebuildTexturesSignalToken_;

	size_t textureSize_ = TextureAtlas::kDefaultAtlasSize;
	TextureGrowthPolicy growthPolicy_ = TextureGrowthPolicy::FlexibleSize;
};