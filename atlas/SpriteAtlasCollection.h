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
	uint32_t generation;
	size_t atlasIndex = std::numeric_limits<size_t>::max();

	bool operator==(const SpriteInfo&) const = default;
};

using SpriteInfoSOA = StableSOA<
	SpriteInfo,
	&SpriteInfo::atlasId,
	&SpriteInfo::plot,
	&SpriteInfo::spriteName,
	&SpriteInfo::filepath,
	&SpriteInfo::generation,
	&SpriteInfo::atlasIndex
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
			SpriteTooLarge,
			SuccessOverwrite
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
	Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize, bool reserved = false);

	Result<SpriteLoadOutcome> 
	LoadSprite(SDL_Renderer* renderer, std::string_view filepath);

	Result<Void> OverwriteSprite(SDL_Renderer* renderer, UniqueSurfacePtr& spriteSurface,
								 AtlasPlot& plot);

	Result<Void> RebuildSourceTexture(SDL_Renderer* renderer, 
									  const SpriteInfoSOA& spriteInfo,
									  size_t& runningIdxCounter);

	bool IsReserved() const noexcept { return isReserved_; }

private:
	explicit SpriteAtlasTexture(TextureAtlasID atlasId) : TextureAtlas(atlasId) {}

	bool isReserved_ = false;
};

class SpriteAtlas : public TextureCreationNotifier
{
public:
	static constexpr size_t kDefaultSpriteInfoCapacity = 50;

	SpriteAtlas() { spriteInfo_.Reserve(kDefaultSpriteInfoCapacity); }
	SpriteAtlas(size_t txSize) : textureSize_(txSize) { spriteInfo_.Reserve(kDefaultSpriteInfoCapacity); }
	SpriteAtlas(size_t txSize, TextureGrowthPolicy policy) : 
		textureSize_(txSize), growthPolicy_(policy) { spriteInfo_.Reserve(kDefaultSpriteInfoCapacity); }
	~SpriteAtlas() = default;

	SpriteAtlas(const SpriteAtlas&) = delete;
	SpriteAtlas& operator=(const SpriteAtlas&) = delete;

	SpriteAtlas(SpriteAtlas&& other) noexcept = default;
	SpriteAtlas& operator=(SpriteAtlas&& other) noexcept = default;

	Result<Sprite> LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor);

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
	bool IsHandleValid(const Handle<TextureResource>& handle) const;

	Result<Void> RebuildSourceTextures(SDL_Renderer* renderer);

	auto GetSpriteInfo(const Handle<TextureResource>& handle) const
	{
		using Ret = decltype(spriteInfo_.TryGetView(0));

		if (!IsHandleValid(handle))
		{
			return Ret{ std::nullopt };
		}

		const size_t spriteIdx = static_cast<size_t>(handle.GetResourceIndex());

		const auto& cInfo = spriteInfo_;

		return cInfo.TryGetView(spriteIdx);
	}
	auto GetSpriteInfo(const Sprite& sprite) const
	{
		return GetSpriteInfo(sprite.resourceHandle);
	}

	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 1)
	auto GetSpriteInfo(const Handle<TextureResource>& handle) const
	{
		using Ret = decltype(spriteInfo_.TryGetView<MemberPtrs...>(0));

		if (!IsHandleValid(handle))
		{
			return Ret{ std::nullopt };
		}

		const size_t spriteIdx = static_cast<size_t>(handle.GetResourceIndex());

		const auto& cInfo = spriteInfo_;

		return cInfo.TryGetView<MemberPtrs...>(spriteIdx);
	}
	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 1)
	auto GetSpriteInfo(const Sprite& sprite) const
	{
		return GetSpriteInfo<MemberPtrs...>(sprite.resourceHandle);
	}

	template <auto MemberPtr>
	auto GetSpriteInfo(const Handle<TextureResource>& handle) const
	{
		using Ret = MonoValueOptionalTupleUnwrapper<
			const typename member_ptr_traits<MemberPtr>::value_type&>;

		if (!IsHandleValid(handle))
		{
			return Ret{ std::nullopt };
		}

		const size_t spriteIdx = static_cast<size_t>(handle.GetResourceIndex());

		const auto& cInfo = spriteInfo_;

		return Ret{ cInfo.TryGetView<MemberPtr>(spriteIdx) };
	}
	template <auto MemberPtr>
	auto GetSpriteInfo(const Sprite& sprite) const
	{
		return GetSpriteInfo<MemberPtr>(sprite.resourceHandle);
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

	template <typename Fn> requires std::invocable<Fn, const Sprite&>
	void ForEachSprite(Fn&& fn) const
	{
		for (size_t i = 0; i < spriteInfo_.Size(); ++i)
		{
			if (!IsPlotEmpty(i))
			{
				std::invoke(fn, MakeSprite(i));
			}
		}
	}

	template <typename Fn> requires std::invocable<Fn, std::string_view, const std::vector<Sprite>&>
	void ForEachSpriteSeries(Fn&& fn) const
	{
		for (const auto& [seriesName, spriteIdxs] : spriteSeriesDefs_)
		{
			auto sprites = spriteIdxs | std::views::transform([this](const auto idx) {
				return MakeSprite(idx);
			}) | std::ranges::to<std::vector>();

			std::invoke(fn, seriesName, sprites);
		}
	}

	//Result<TextureAtlasID> ReserveTexture(SDL_Renderer* renderer);
	//Result<TextureAtlasID> ReserveTexture(SDL_Renderer* renderer, size_t size);

	Result<Void> DefineSpriteSeries(std::string_view seriesName, std::span<const Sprite> sprites);
	bool RemoveSpriteSeries(std::string_view seriesName);
	bool RemoveSpriteSeriesMember(std::string_view seriesName, const Sprite& sprite);
	size_t GetSpriteSeriesMemberIndex(std::string_view seriesName, const Sprite& sprite) const;
	std::vector<std::string_view> GetSpriteSeriesNames() const;
	//const UnorderedDictionary<std::vector<size_t>> 
	//GetSpriteSeriesDefintions() const { return spriteSeriesDefs_; }

	bool EraseSprite(const Sprite& sprite);

	size_t GetTextureCount() const;
	size_t GetSpriteCount() const;

	size_t GetTextureSize() const;
	void SetTextureSize(size_t newSize);

	TextureGrowthPolicy GetTextureGrowthPolicy() const;
	void SetTextureGrowthPolicy(TextureGrowthPolicy newPolicy);

	SpriteDescriptorPackage ExportSpriteDescriptors() const;
	SerializedSpriteDescriptorPackage Serialize() const;
	Result<Void> Deserialize(SDL_Renderer* renderer, SerializedSpriteDescriptorPackage&& package);

	Result<Void> CopyContentsFrom(const SpriteAtlas& other, SDL_Renderer* renderer);

	SDL_Texture* GetSpriteSourceTexture(const Sprite& sprite);

private:
	Result<std::pair<Sprite, size_t>> 
	LoadSpriteImpl(SDL_Renderer* renderer, std::string&& filepath, std::string&& spriteName);
	//Result<Sprite> LoadSpriteImpl(SDL_Renderer* renderer, std::string filepath,
	//							  std::string spriteName);

	Result<std::vector<Sprite>> LoadSpritesImpl(SDL_Renderer* renderer,
												SpriteDescriptors&& descriptors);

	Sprite MakeSprite(size_t spriteIndex) const;

	size_t FindSuitableFreePlotIndex(int spriteW, int spriteH) const;

	Result<std::pair<Sprite, size_t>> 
	OverwriteSprite(SDL_Renderer* renderer, UniqueSurfacePtr& spriteSurface, 
					std::string&& filepath, std::string&& spriteName, size_t freePlotIdx);

	Result<Void> AddNewAtlasTexture(SDL_Renderer* renderer);

	bool IsPlotEmpty(size_t spriteInfoIdx) const;

	TextureAtlasID GetTextureAtlasIdForSpriteIndex(size_t ) const;

	std::vector<SpriteAtlasTexture> spriteAtlasTextures_;
	SpriteInfoSOA spriteInfo_;
	UnorderedDictionary<size_t> spriteNameIndices_;
	UnorderedDictionary<std::vector<size_t>> spriteSeriesDefs_;
	SignalToken rebuildTexturesSignalToken_;

	std::vector<size_t> freePlots_;

	size_t textureSize_ = TextureAtlas::kDefaultAtlasSize;
	TextureGrowthPolicy growthPolicy_ = TextureGrowthPolicy::FlexibleSize;
};