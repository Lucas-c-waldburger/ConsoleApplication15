#include "SpriteAtlasCollection.h"
#include "../core/Hash.h"
#include "../core/ScopedInvoker.h"
#include "../events/EventBus2.h"
#include <filesystem>
#include <SDL_image.h>
#include <ranges>

namespace {

constexpr bool IsPowerOfTwo(size_t x) noexcept
{
    return x != 0 && (x & (x - 1)) == 0;
}

} // unnamed

/** @defgroup SpriteAtlasTexture @{ */

//// TODO:
    // replace atlasId with atlasIndex in SpriteInfo - get atlasId when needed using atlasIndex
    // 

SpriteAtlasTexture::SpriteAtlasTexture(SpriteAtlasTexture&& other) noexcept : 
	TextureAtlas(std::move(other)), isReserved_(other.isReserved_)
{}

SpriteAtlasTexture& SpriteAtlasTexture::operator=(SpriteAtlasTexture&& other) noexcept
{
    if (this != &other)
    {
        TextureAtlas::operator=(std::move(other));
		isReserved_ = other.isReserved_;
    }
    return *this;
}

Result<SpriteAtlasTexture>
SpriteAtlasTexture::Create(SDL_Renderer* renderer, size_t size, bool reserved)
{
    SpriteAtlasTexture atlas{ GetNextAtlasID() };

    atlas.textureSize_ = IsPowerOfTwo(size) ? size :
        size > kMaxAtlasSize ? kMaxAtlasSize :
        GetNextPowerOfTwo(static_cast<int>(size));

    atlas.atlasTexture_ = MakeUniqueTexturePtr(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size
    );
    if (!atlas.atlasTexture_)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    SDL_SetTextureBlendMode(atlas.atlasTexture_.get(), SDL_BLENDMODE_BLEND);

    atlas.binPack_.Init(size, size, false);

    atlas.isReserved_ = reserved;

    return atlas;
}

Result<SpriteAtlasTexture::SpriteLoadOutcome> 
SpriteAtlasTexture::LoadSprite(SDL_Renderer* renderer, std::string_view filepath)
{
    if (!IsLoaded())
    {
        return MAKE_ERROR("Atlas was not loaded");
    }

    SDL_Surface* spriteSurface = nullptr;
    SDL_Texture* spriteTexture = nullptr;

    ScopedInvoker freeResources{ [&] {
        SDL_FreeSurface(spriteSurface);
        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
    } };

    spriteSurface = IMG_Load(filepath.data());
    if (!spriteSurface)
    {
        return MAKE_ERROR(IMG_GetError());
    }

    if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
    {
        return MAKE_ERROR_FMT("Invalid surface dimensions: ({}, {})",
            spriteSurface->w, spriteSurface->h);
    }
    if (static_cast<size_t>(spriteSurface->w) > GetTextureSize() || 
        static_cast<size_t>(spriteSurface->h) > GetTextureSize())
    {
		return SpriteLoadOutcome{ .code = SpriteLoadOutcome::SpriteTooLarge };
	}

    AtlasPlot plot{ .rotation = 0.0f };

    rbp::Rect packed = binPack_.Insert(spriteSurface->w, spriteSurface->h,
        rbp::MaxRectsBinPack::RectBestAreaFit);
    if (!WasRectPacked(packed))
    {
        return SpriteLoadOutcome{ .code = SpriteLoadOutcome::AtlasFull };
    }

    plot.rect = RbpToSDLRect(packed);

    spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurface);
    if (!spriteTexture)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    if (SDL_RenderCopy(renderer, spriteTexture, nullptr, &plot.rect) != 0)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    return SpriteLoadOutcome{
		.code = SpriteLoadOutcome::Success,
        .spriteInfo = {
            .atlasId = GetAtlasID(),
            .plot = plot
        }
    };
}

Result<Void> SpriteAtlasTexture::OverwriteSprite(SDL_Renderer* renderer, UniqueSurfacePtr& spriteSurface,
                                                 AtlasPlot& plot)
{
    if (!IsLoaded())
    {
        return MAKE_ERROR("Atlas was not loaded");
    }

    assert(spriteSurface);

    auto spriteTexture = MakeUniqueTexturePtrFromSurface(renderer, spriteSurface.get());
    if (!spriteTexture)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    if (SDL_RenderCopy(renderer, spriteTexture.get(), nullptr, &plot.rect) != 0)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    return kVoid;
}

Result<Void> SpriteAtlasTexture::RebuildSourceTexture(SDL_Renderer* renderer,
                                                  const SpriteInfoSOA& spriteInfo, 
                                                  size_t& runningIdxCounter)
{
    SDL_DestroyTexture(atlasTexture_.get());

    atlasTexture_ = MakeUniqueTexturePtr(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        GetTextureSize(), GetTextureSize()
    );
    if (!atlasTexture_)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    SDL_SetTextureBlendMode(atlasTexture_.get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, atlasTexture_.get());

    SDL_Surface* spriteSurface = nullptr;
    SDL_Texture* spriteTexture = nullptr;

    ScopedInvoker freeResources{ [&] {
        SDL_FreeSurface(spriteSurface);
        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
        SDL_SetRenderTarget(renderer, nullptr);
    } };

    auto run = [&] {
        return runningIdxCounter < spriteInfo.Size() &&
               spriteInfo.GetView<&SpriteInfo::atlasId>(runningIdxCounter) == 
               GetAtlasID();
    };

    while (run())
    {
        const auto [plot, path] = spriteInfo.GetView<&SpriteInfo::plot,
                                                     &SpriteInfo::filepath>
                                                     (runningIdxCounter);

        spriteSurface = IMG_Load(path.c_str());
        if (!spriteSurface)
        {
            return MAKE_ERROR(IMG_GetError());
        }

        if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
        {
            return MAKE_ERROR_FMT("Invalid surface dimensions: ({}, {})",
                spriteSurface->w, spriteSurface->h);
        }

        spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurface);
        if (!spriteTexture)
        {
            return MAKE_ERROR(SDL_GetError());
        }
        SDL_FreeSurface(spriteSurface);

        if (SDL_RenderCopy(renderer, spriteTexture, nullptr, &plot.rect) != 0)
        {
            return MAKE_ERROR(SDL_GetError());
        }
        SDL_DestroyTexture(spriteTexture);

        ++runningIdxCounter;
    }

    return kVoid;
}
/** @} */

/** @defgroup SpriteAtlas @{ */
Result<Sprite> SpriteAtlas::LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
{
    if (spriteAtlasTextures_.empty())
    {
        TRY(AddNewAtlasTexture(renderer));
    }
    assert(!spriteAtlasTextures_.empty());

    //SDL_SetRenderTarget(renderer, spriteAtlasTextures_.back().GetSourceTexture());

    auto loadResult = LoadSpriteImpl(renderer, 
        std::move(descriptor.filepath), std::move(descriptor.spriteName));

    SDL_SetRenderTarget(renderer, nullptr);

    if (!loadResult.Success())
    {
        return loadResult.GetError();
    }

    return loadResult.GetValue().first;
}

//Result<TextureAtlasID> SpriteAtlas::ReserveTexture(SDL_Renderer* renderer, size_t size)
//{
//    TRY_ASSIGN(spriteAtlasTextures_.emplace_back(), SpriteAtlasTexture::Create(renderer, size, true));
//
//    return spriteAtlasTextures_.back().GetAtlasID();
//}
//
//Result<TextureAtlasID> SpriteAtlas::ReserveTexture(SDL_Renderer* renderer)
//{
//    return ReserveTexture(renderer, textureSize_);
//}

Result<Void> SpriteAtlas::DefineSpriteSeries(std::string_view seriesName, std::span<const Sprite> sprites)
{
    if (spriteSeriesDefs_.contains(seriesName))
    {
        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists", seriesName);
    }

    auto& indices = spriteSeriesDefs_[seriesName];
    indices.reserve(sprites.size());

    for (size_t i = 0; i < sprites.size(); ++i)
    {
        if (!IsSpriteValid(sprites[i]))
        {
            return MAKE_ERROR_FMT("Sprite at index '{}' was invalid", i);
        }

        indices.emplace_back(sprites[i].resourceHandle.GetResourceIndex());
    }

    return kVoid;
}

bool SpriteAtlas::RemoveSpriteSeries(std::string_view seriesName)
{
    return spriteSeriesDefs_.erase(seriesName) > 0;
}

bool SpriteAtlas::RemoveSpriteSeriesMember(std::string_view seriesName, const Sprite& sprite)
{
    auto it = spriteSeriesDefs_.find(seriesName);
    if (it == spriteSeriesDefs_.end())
    {
        return false;
    }

    return core::Erase(it->second, sprite.resourceHandle.GetResourceIndex());
}

size_t SpriteAtlas::GetSpriteSeriesMemberIndex(std::string_view seriesName, const Sprite& sprite) const
{
    auto it = spriteSeriesDefs_.find(seriesName);
    if (it == spriteSeriesDefs_.end())
    {
        return std::numeric_limits<size_t>::max();
    }

	const auto spriteIdx = static_cast<size_t>(sprite.resourceHandle.GetResourceIndex());
	auto foundIt = core::Find(it->second, spriteIdx);

    return (foundIt != it->second.end()) 
        ? static_cast<size_t>(std::distance(it->second.begin(), foundIt)) 
        : std::numeric_limits<size_t>::max();
}

bool SpriteAtlas::EraseSprite(const Sprite& sprite)
{
    if (!IsSpriteValid(sprite))
    {
        return false;
    }

    const auto spriteIdx = sprite.resourceHandle.GetResourceIndex();
    auto [filepath, spriteName, plot, gen] = spriteInfo_.GetView<&SpriteInfo::filepath,
                                                                 &SpriteInfo::spriteName,
                                                                 &SpriteInfo::plot,
                                                                 &SpriteInfo::generation>(spriteIdx);
    assert(!filepath.empty());
    assert(!spriteName.empty());
    assert(plot.rect.w > 0 && plot.rect.h > 0);

    [[maybe_unused]] const size_t nameErased = spriteNameIndices_.erase(spriteName);
    assert(nameErased > 0);

    filepath.clear();
    spriteName.clear();
    ++gen;

    freePlots_.emplace_back(spriteIdx);

    for (auto& [_, seriesMemberIdx] : spriteSeriesDefs_)
    {
        core::Erase(seriesMemberIdx, spriteIdx);
	}

    return true;
}

std::vector<std::string_view> SpriteAtlas::GetSpriteSeriesNames() const
{
    return spriteSeriesDefs_ | std::views::transform([](const auto& pair) {
        return std::string_view{ pair.first };
    }) | std::ranges::to<std::vector>();
}

size_t SpriteAtlas::GetTextureCount() const
{
    return spriteAtlasTextures_.size();
}

size_t SpriteAtlas::GetSpriteCount() const 
{
	return spriteInfo_.Size() - freePlots_.size();
}

size_t SpriteAtlas::GetTextureSize() const
{
    return textureSize_;
}

void SpriteAtlas::SetTextureSize(size_t size)
{
    textureSize_ = size;
}

auto SpriteAtlas::GetTextureGrowthPolicy() const -> TextureGrowthPolicy
{
    return growthPolicy_;
}

void SpriteAtlas::SetTextureGrowthPolicy(TextureGrowthPolicy policy)
{
    growthPolicy_ = policy;
}

Result<Void> SpriteAtlas::CopyContentsFrom(const SpriteAtlas& other, SDL_Renderer* renderer)
{
    if (other.GetSpriteCount() == 0)
    {
        return kVoid;
    }

    // store mapping of spriteInfo index from other to the 
    // new index it gets assigned in this atlas below
    auto oldSpriteInfoIdxToNew = other.spriteSeriesDefs_
    | std::views::values
    | std::views::join
    | std::views::transform([](size_t spriteInfoIdx) {
        return std::pair<size_t, size_t>{ spriteInfoIdx, std::numeric_limits<size_t>::max() };
    }) | std::ranges::to<std::unordered_map<size_t, size_t>>();

    // load all valid sprites from other into this atlas
    for (size_t i = 0; i < other.spriteInfo_.Size(); ++i)
    {
        if (other.IsPlotEmpty(i))
        {
            continue;
        }

        const auto [filepath, spriteName] = other.spriteInfo_.GetView<&SpriteInfo::filepath,
                                                                      &SpriteInfo::spriteName>(i);

        if (spriteNameIndices_.contains(spriteName))
        {
            LOG_DEBUG_FMT("Skipping duplicate sprite name '{}'", spriteName);

            continue;
        }

        auto tmpFilepath = filepath;
        auto tmpSpriteName = spriteName;

        TRY(LoadSpriteImpl(renderer, std::move(tmpFilepath), std::move(tmpSpriteName)),
            result);

        // if this sprite is part of series in other, have the old index point to 
        // where it now lives inside this atlas
        if (auto it = oldSpriteInfoIdxToNew.find(i); it != oldSpriteInfoIdxToNew.end())
        {
            it->second = result.second;
        }
    }

    for (const auto& [seriesName, spriteInfoIdxs] : other.spriteSeriesDefs_)
    {
        if (spriteSeriesDefs_.contains(seriesName))
        {
            LOG_DEBUG_FMT("Skipping duplicate sprite series name '{}'", seriesName);

            continue;
        }

        auto [seriesDefIt, _] = spriteSeriesDefs_.emplace(seriesName, spriteInfoIdxs);
        auto& seriesDefSpriteIdxs = seriesDefIt->second;

        // iterate through other sprite series defs, reassign indices if valid
        auto spriteInfoIdxIt = seriesDefSpriteIdxs.begin();
        while (spriteInfoIdxIt != seriesDefSpriteIdxs.end())
        {
            auto it = oldSpriteInfoIdxToNew.find(*spriteInfoIdxIt);
            if (it != oldSpriteInfoIdxToNew.end() && it->second != std::numeric_limits<size_t>::max())
            {
                *spriteInfoIdxIt = it->second;
                ++spriteInfoIdxIt;
            }
            else
            {
                spriteInfoIdxIt = seriesDefIt->second.erase(spriteInfoIdxIt);
            }
        }
    }

    return kVoid;
}

Result<std::pair<Sprite, size_t>> 
SpriteAtlas::OverwriteSprite(SDL_Renderer* renderer, UniqueSurfacePtr& spriteSurface, 
                             std::string&& filepath, std::string&& spriteName, size_t freePlotIdx)
{
    assert(freePlotIdx < freePlots_.size());
    const auto spriteIdx = freePlots_[freePlotIdx];

    auto [emptyFilepath, emptySpriteName, plot, atlasIndex] = spriteInfo_.GetView<
        &SpriteInfo::filepath,
        &SpriteInfo::spriteName,
        &SpriteInfo::plot,
        &SpriteInfo::atlasIndex>(spriteIdx);

    assert(atlasIndex < spriteAtlasTextures_.size());
    assert(emptyFilepath.empty());
    assert(emptySpriteName.empty());
    assert(plot.rect.w > 0 && plot.rect.h > 0);

    SDL_SetRenderTarget(renderer, spriteAtlasTextures_[atlasIndex].GetSourceTexture());

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Rect resizedRect{
        .x = plot.rect.x,
        .y = plot.rect.y,
        .w = spriteSurface->w,
        .h = spriteSurface->h
    };

    if (SDL_RenderFillRect(renderer, &resizedRect) < 0)
    {
        SDL_SetRenderTarget(renderer, nullptr);

        return MAKE_ERROR(SDL_GetError());
    }

    SDL_SetRenderTarget(renderer, spriteAtlasTextures_[atlasIndex].GetSourceTexture());

    TRY(spriteAtlasTextures_[atlasIndex].OverwriteSprite(renderer, spriteSurface, plot));

    emptyFilepath = std::move(filepath);
    emptySpriteName = std::move(spriteName);
    plot.rect = resizedRect;

    const auto backIdx = freePlots_.size() - 1;
    freePlots_[freePlotIdx] = freePlots_[backIdx];
    freePlots_.pop_back();

    return std::make_pair(MakeSprite(spriteIdx), spriteIdx);
}

Result<std::pair<Sprite, size_t>>  
SpriteAtlas::LoadSpriteImpl(SDL_Renderer* renderer, std::string&& filepath, std::string&& spriteName)
{
    if (!std::filesystem::exists(filepath))
    {
        return MAKE_ERROR_FMT("Invalid filepath: '{}'", filepath);
    }

    if (spriteName.empty())
    {
        spriteName = std::filesystem::path(filepath).stem().string();
    }

    if (auto it = spriteNameIndices_.find(spriteName);
        it != spriteNameIndices_.end())
    {
        LOG_INFO_FMT("Sprite name '{}' already exists in atlas. Returning original sprite",
            spriteName);

        return std::make_pair(MakeSprite(it->second), it->second);
    }

    auto spriteSurface = MakeUniqueSurfacePtr(filepath);
    if (!spriteSurface)
    {
        return MAKE_ERROR(IMG_GetError());
    }
    if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
    {
        return MAKE_ERROR_FMT("Invalid surface dimensions: ({}, {})",
            spriteSurface->w, spriteSurface->h);
    }

    // if large-enough plot available, overwrite it
    const auto freePlotIdx = FindSuitableFreePlotIndex(spriteSurface->w, spriteSurface->h);
    if (freePlotIdx < freePlots_.size())
    {
        return OverwriteSprite(renderer, spriteSurface, std::move(filepath), 
                               std::move(spriteName), freePlotIdx);
    }

    // else no free plot to reuse - make new one
    using Outcome = SpriteAtlasTexture::SpriteLoadOutcome;
    Outcome loadOutcome{};

    size_t originalTextureSize = textureSize_;
    SDL_Texture* renderTarget = nullptr;

    do
    {
        //// TODO: Can we move this out and above for performance if loading multiple sprites at once?
        SDL_SetRenderTarget(renderer, spriteAtlasTextures_.back().GetSourceTexture());

        TRY_ASSIGN(loadOutcome, spriteAtlasTextures_.back().LoadSprite(renderer, filepath));

        if (loadOutcome.code == Outcome::SpriteTooLarge)
        {
            if (growthPolicy_ == TextureGrowthPolicy::FixedSize ||
                textureSize_ >= TextureAtlas::kMaxAtlasSize)
            {
                return MAKE_ERROR_FMT("Sprite '{}' was too large to fit in atlas texture",
                    filepath);
            }
            else
            {
                textureSize_ = std::min(textureSize_ * 2, TextureAtlas::kMaxAtlasSize);
                loadOutcome.code = Outcome::AtlasFull;
            }
        }
        if (loadOutcome.code == Outcome::AtlasFull)
        {
            TRY(AddNewAtlasTexture(renderer));
        }

    } while (loadOutcome.code != Outcome::Success);

    textureSize_ = originalTextureSize;

    auto& newSpriteInfo = loadOutcome.spriteInfo;
    newSpriteInfo.spriteName = spriteName;
    newSpriteInfo.filepath = std::move(filepath);
	newSpriteInfo.atlasIndex = spriteAtlasTextures_.size() - 1;
	newSpriteInfo.generation = 0;

    size_t spriteIdx = spriteInfo_.PushBack(std::move(newSpriteInfo));

    auto [_, inserted] = spriteNameIndices_.try_emplace(std::move(spriteName), spriteIdx);
    assert(inserted);

    [[maybe_unused]] const auto& plot = spriteInfo_.GetView<&SpriteInfo::plot>(spriteIdx);
    assert(plot.rect.w > 0 && plot.rect.h > 0);

    const auto resourceHandle = Handle<TextureResource>::Create(
        spriteAtlasTextures_.back().GetAtlasID(), spriteIdx, 0
    );

    return std::make_pair(Sprite{
        .resourceHandle = resourceHandle,
        .plot = plot
    }, spriteIdx);
}

//Result<Sprite> SpriteAtlas::LoadSpriteImpl(SDL_Renderer* renderer, std::string filepath,
//                                           std::string spriteName)
//{
//    return LoadSpriteImpl(renderer, std::move(filepath), std::move(spriteName));
//}

size_t SpriteAtlas::FindSuitableFreePlotIndex(int spriteW, int spriteH) const
{
    assert(spriteW > 0 && spriteH > 0);

    size_t overwriteIndex = std::numeric_limits<size_t>::max();
    int leastAreaWaste = std::numeric_limits<int>::max();
    for (size_t i = 0; i < freePlots_.size(); ++i)
    {
        const auto idx = freePlots_[i];

        assert(idx < spriteInfo_.Size());

        const auto [plot, atlasId] = spriteInfo_.GetView<&SpriteInfo::plot, &SpriteInfo::atlasId>(idx);
        if (spriteW > plot.rect.w || spriteH > plot.rect.h)
        {
            continue;
        }

        const int areaWaste = plot.rect.w * plot.rect.h - spriteW * spriteH;
        if (areaWaste < leastAreaWaste)
        {
            leastAreaWaste = areaWaste;
            overwriteIndex = i;
        }
    }

    return overwriteIndex;
}

Result<Void> SpriteAtlas::AddNewAtlasTexture(SDL_Renderer* renderer)
{
    assert(textureSize_ > 0);

    TRY_ASSIGN(spriteAtlasTextures_.emplace_back(),
        SpriteAtlasTexture::Create(renderer, textureSize_));

    auto& newAtlas = spriteAtlasTextures_.back();

    NotifyTextureCreated(newAtlas.GetAtlasID(), newAtlas.GetSourceTexture());

    return kVoid;
}

bool SpriteAtlas::IsPlotEmpty(size_t spriteInfoIdx) const
{
    assert(spriteInfoIdx < spriteInfo_.Size());

    const auto [filepath, name] = spriteInfo_.GetView<&SpriteInfo::filepath,
                                                      &SpriteInfo::spriteName>(spriteInfoIdx);
    if (filepath.empty())
    {
        assert(name.empty());

        return true;
    }

    return false;
}

TextureAtlasID SpriteAtlas::GetTextureAtlasIdForSpriteIndex(size_t spriteIdx) const
{
    if (spriteIdx >= spriteInfo_.Size())
    {
        return std::numeric_limits<uint32_t>::max();
	}

	const auto atlasIdx = spriteInfo_.GetView<&SpriteInfo::atlasIndex>(spriteIdx);

    return (atlasIdx < spriteAtlasTextures_.size()) 
        ? spriteAtlasTextures_[atlasIdx].GetAtlasID() 
		: std::numeric_limits<uint32_t>::max();
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSprites(SDL_Renderer* renderer, SpriteDescriptors&& descriptors)
{
    if (descriptors.data.empty())
    {
        return MAKE_ERROR("Sprite descriptors were empty");
    }

    if (!descriptors.seriesName.empty() && spriteSeriesDefs_.contains(descriptors.seriesName))
    {
        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists",
            descriptors.seriesName);
    }

    spriteInfo_.Reserve(spriteInfo_.Size() + descriptors.data.size());

    if (spriteAtlasTextures_.empty())
    {
        TRY(AddNewAtlasTexture(renderer));
    }
    assert(!spriteAtlasTextures_.empty());

    //SDL_SetRenderTarget(renderer, spriteAtlasTextures_.back().GetSourceTexture());

    auto loadResult = LoadSpritesImpl(renderer, std::move(descriptors));

    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Sprite SpriteAtlas::GetSprite(std::string_view spriteName) const
{
    auto it = spriteNameIndices_.find(spriteName);
    if (it == spriteNameIndices_.end())
    {
        return {};
    }

    assert(it->second < spriteInfo_.Size());
    assert(spriteInfo_.GetView<&SpriteInfo::spriteName>(it->second) == spriteName);

    return MakeSprite(it->second);
}

Sprite SpriteAtlas::GetSprite(const Handle<TextureResource>& handle) const
{
    const auto spriteIdx = static_cast<size_t>(handle.GetResourceIndex());
    if (spriteIdx >= spriteInfo_.Size())
    {
        return {};
    }

    assert(handle.GetAtlasID() == 
        spriteInfo_.GetView<&SpriteInfo::atlasId>(spriteIdx));

    return MakeSprite(spriteIdx);
}

std::vector<Sprite> 
SpriteAtlas::GetSpriteSeries(std::string_view seriesName) const
{
    auto it = spriteSeriesDefs_.find(seriesName);
    if (it == spriteSeriesDefs_.end())
    {
        return {};
    }

    return it->second | std::views::transform([this](const auto idx) {
        return MakeSprite(idx);
     }) | std::ranges::to<std::vector>();
}

Sprite SpriteAtlas::GetSpriteSeriesMember(std::string_view seriesName, 
                                          size_t idxInSeries) const
{
    auto it = spriteSeriesDefs_.find(seriesName);
    if (it == spriteSeriesDefs_.end())
    {
        return {};
    }

    if (idxInSeries >= it->second.size())
    {
        return {};
    }

    return MakeSprite(it->second[idxInSeries]);
}

size_t SpriteAtlas::GetSpriteSeriesSize(std::string_view seriesName) const
{
    auto it = spriteSeriesDefs_.find(seriesName);

    return (it != spriteSeriesDefs_.end()) ? it->second.size() : 0_uz;
}

bool SpriteAtlas::HasSprite(std::string_view spriteName) const
{
    return spriteNameIndices_.contains(spriteName);
}

bool SpriteAtlas::HasSpriteSeries(std::string_view seriesName) const
{
    return spriteSeriesDefs_.contains(seriesName);
}

bool SpriteAtlas::IsSpriteValid(const Sprite& sprite) const
{
    const auto spriteIdx = static_cast<size_t>(
        sprite.resourceHandle.GetResourceIndex());

    if (spriteIdx >= spriteInfo_.Size())
    {
        return false;
    }

    const auto [atlasId, plot, gen] = spriteInfo_.GetView<&SpriteInfo::atlasId,
                                                          &SpriteInfo::plot,
                                                          &SpriteInfo::generation>(spriteIdx);

    return sprite.resourceHandle.GetAtlasID() == atlasId && 
           sprite.resourceHandle.GetGeneration() == gen &&
           sprite.plot == plot;
}

bool SpriteAtlas::IsHandleValid(const Handle<TextureResource>& handle) const
{
    const auto spriteIdx = static_cast<size_t>(handle.GetResourceIndex());
    if (spriteIdx >= spriteInfo_.Size())
    {
        return false;
    }

    const auto [atlasId, gen] = spriteInfo_.GetView<&SpriteInfo::atlasId,
                                                    &SpriteInfo::generation>(spriteIdx);

    return handle.GetAtlasID() == atlasId && handle.GetGeneration() == gen;
}

Result<Void> SpriteAtlas::RebuildSourceTextures(SDL_Renderer* renderer)
{
    size_t runningIdxCounter = 0;

    for (auto& spriteAtlas : spriteAtlasTextures_)
    {
        TRY(spriteAtlas.RebuildSourceTexture(renderer, spriteInfo_, runningIdxCounter));

        NotifyTextureCreated(spriteAtlas.GetAtlasID(), spriteAtlas.GetSourceTexture());
    }

    assert(runningIdxCounter == spriteInfo_.Size());

    return kVoid;
}

Sprite SpriteAtlas::MakeSprite(size_t spriteIndex) const
{
    if (IsPlotEmpty(spriteIndex))
    {
        return {};
    }

    const auto [atlasId, plot, gen] = spriteInfo_.GetView<&SpriteInfo::atlasId,
                                                          &SpriteInfo::plot,
                                                          &SpriteInfo::generation>(spriteIndex);

    return Sprite{
        .resourceHandle = Handle<TextureResource>::Create(atlasId, spriteIndex, gen),
        .plot = plot
    };
}

Result<std::vector<Sprite>>
SpriteAtlas::LoadSpritesImpl(SDL_Renderer* renderer,
                             SpriteDescriptors&& descriptors)
{
    std::vector<Sprite> sprites;
    sprites.reserve(descriptors.data.size());

    const bool isSpriteSeries = !descriptors.seriesName.empty();
    if (isSpriteSeries)
    {
        spriteSeriesDefs_[descriptors.seriesName].reserve(descriptors.data.size());
    }

    for (auto&& descriptor : descriptors.data)
    {
        TRY(LoadSpriteImpl(renderer, std::move(descriptor.filepath), 
            std::move(descriptor.spriteName)), result);

        sprites.emplace_back(result.first);

        if (isSpriteSeries)
        {
            spriteSeriesDefs_[descriptors.seriesName].emplace_back(
                sprites.back().resourceHandle.GetResourceIndex()
            );
        }
    }

    return sprites;
}

SerializedSpriteDescriptorPackage SpriteAtlas::Serialize() const
{
    SerializedSpriteDescriptorPackage package;
    package.spriteData.reserve(GetSpriteCount());

	// store mapping of spriteInfo index to package.spriteData index for use by series definitions
	auto spriteInfoIdxToDescriptorIdx = spriteSeriesDefs_ 
        | std::views::values 
        | std::views::join 
        | std::views::transform([](size_t spriteInfoIdx) { 
            return std::pair<size_t, size_t>{ spriteInfoIdx, std::numeric_limits<size_t>::max() };
        }) | std::ranges::to<std::unordered_map<size_t, size_t>>();

    for (size_t i = 0; i < spriteInfo_.Size(); ++i)
    {
        if (IsPlotEmpty(i))
        {
            continue;
        }

        // store sprite descriptor data
        const auto [spriteName, filepath] = spriteInfo_.GetView<&SpriteInfo::spriteName,
                                                                &SpriteInfo::filepath>(i);

        package.spriteData.emplace_back(spriteName, filepath);

		// if this sprite index is part of a series, store the mapping to the package index
        if (spriteInfoIdxToDescriptorIdx.contains(i))
        {
            spriteInfoIdxToDescriptorIdx[i] = package.spriteData.size() - 1;
		}
    }

	// store series definitions
    package.seriesDefinitions.reserve(spriteSeriesDefs_.size());
    for (const auto& [seriesName, spriteInfoIndices] : spriteSeriesDefs_)
    {
        if (spriteInfoIndices.empty())
        {
            continue;
        }

        auto& seriesDef = package.seriesDefinitions.emplace_back();
        seriesDef.seriesName = seriesName;
        seriesDef.spriteDataIndices.reserve(spriteInfoIndices.size());

        for (const auto& spriteInfoIdx : spriteInfoIndices)
        {
            const auto it = spriteInfoIdxToDescriptorIdx.find(spriteInfoIdx);
            assert(it != spriteInfoIdxToDescriptorIdx.end());
            assert(it->second != std::numeric_limits<size_t>::max());

            seriesDef.spriteDataIndices.emplace_back(it->second);
        }
	}

    return package;
}

Result<Void> SpriteAtlas::Deserialize(SDL_Renderer* renderer, SerializedSpriteDescriptorPackage&& package)
{
    assert(renderer);

    if (package.spriteData.empty())
    {
        return kVoid;
    }

    // store mapping of package.spriteData index to assigned spriteInfo index
    std::unordered_map<size_t, size_t> descriptorIdxToSpriteInfoIdx;
    for (const auto& seriesDef : package.seriesDefinitions)
    {
        for (const size_t descriptorIdx : seriesDef.spriteDataIndices)
        {
            descriptorIdxToSpriteInfoIdx.try_emplace(descriptorIdx, std::numeric_limits<size_t>::max());
        }
    }

    TRY(AddNewAtlasTexture(renderer));

    spriteNameIndices_.reserve(package.spriteData.size());

    // load sprites
    for (size_t descriptorIdx = 0; descriptorIdx < package.spriteData.size(); ++descriptorIdx)
    {
        auto& filepath = package.spriteData[descriptorIdx].filepath;
        auto& spriteName = package.spriteData[descriptorIdx].spriteName;

        if (!std::filesystem::exists(filepath))
        {
            return MAKE_ERROR_FMT("Invalid filepath: '{}'", filepath);
        }

        if (spriteName.empty())
        {
            return MAKE_ERROR_FMT("Sprite with filepath '{}' did not have a name", filepath);
        }

        if (spriteNameIndices_.contains(spriteName))
        {
            return MAKE_ERROR_FMT("Duplicate sprite name: '{}'", spriteName);
        }

        auto spriteSurface = MakeUniqueSurfacePtr(filepath);
        if (!spriteSurface)
        {
            return MAKE_ERROR(IMG_GetError());
        }
        if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
        {
            return MAKE_ERROR_FMT("Invalid surface dimensions for sprite '{}': ({}, {})",
                spriteName, spriteSurface->w, spriteSurface->h);
        }

        using Outcome = SpriteAtlasTexture::SpriteLoadOutcome;
        Outcome loadOutcome{};
        size_t originalTextureSize = textureSize_;

        do
        {
            SDL_SetRenderTarget(renderer, spriteAtlasTextures_.back().GetSourceTexture());

            TRY_ASSIGN(loadOutcome, spriteAtlasTextures_.back().LoadSprite(renderer, filepath));

            if (loadOutcome.code == Outcome::SpriteTooLarge)
            {
                if (growthPolicy_ == TextureGrowthPolicy::FixedSize ||
                    textureSize_ >= TextureAtlas::kMaxAtlasSize)
                {
                    return MAKE_ERROR_FMT("Sprite '{}' was too large to fit in atlas texture",
                        spriteName);
                }
                else
                {
                    textureSize_ = std::min(textureSize_ * 2, TextureAtlas::kMaxAtlasSize);
                    loadOutcome.code = Outcome::AtlasFull;
                }
            }
            if (loadOutcome.code == Outcome::AtlasFull)
            {
                TRY(AddNewAtlasTexture(renderer));
            }

        } while (loadOutcome.code != Outcome::Success);

        textureSize_ = originalTextureSize;

        auto& newSpriteInfo = loadOutcome.spriteInfo;
        newSpriteInfo.spriteName = spriteName;
        newSpriteInfo.filepath = std::move(filepath);
        newSpriteInfo.atlasIndex = spriteAtlasTextures_.size() - 1;
        newSpriteInfo.generation = 0;

        size_t spriteInfoIdx = spriteInfo_.PushBack(std::move(newSpriteInfo));

        auto [_, inserted] = spriteNameIndices_.try_emplace(std::move(spriteName), spriteInfoIdx);
        assert(inserted);

        [[maybe_unused]] const auto& plot = spriteInfo_.GetView<&SpriteInfo::plot>(spriteInfoIdx);
        assert(plot.rect.w > 0 && plot.rect.h > 0);

        // if this sprite is part of a series, store the mapping to its new spriteInfo index
        if (descriptorIdxToSpriteInfoIdx.contains(descriptorIdx))
        {
            descriptorIdxToSpriteInfoIdx[descriptorIdx] = spriteInfoIdx;
        }
    }

    // resolve sprite series definitions
    spriteSeriesDefs_.reserve(package.seriesDefinitions.size());
    for (auto&& [packageSeriesName, packageSpriteDescriptorIdxs] : package.seriesDefinitions)
    {
        if (spriteSeriesDefs_.contains(packageSeriesName))
        {
            return MAKE_ERROR_FMT("Duplicate sprite series name: '{}'", packageSeriesName);
        }

        auto [_, inserted] = spriteSeriesDefs_.try_emplace(
            std::move(packageSeriesName), std::move(packageSpriteDescriptorIdxs)
        );
        assert(inserted);

        for (auto& packageSpriteDescriptorIdx : packageSpriteDescriptorIdxs)
        {
            const auto it = descriptorIdxToSpriteInfoIdx.find(packageSpriteDescriptorIdx);
            assert(it != descriptorIdxToSpriteInfoIdx.end());
            assert(it->second != std::numeric_limits<size_t>::max());

            packageSpriteDescriptorIdx = it->second;
        }
    }

    return kVoid;
}

SDL_Texture* SpriteAtlas::GetSpriteSourceTexture(const Sprite& sprite)
{
    if (!IsSpriteValid(sprite))
    {
        return nullptr;
    }

    const auto atlasIndex = spriteInfo_.GetView<&SpriteInfo::atlasIndex>(
        static_cast<size_t>(sprite.resourceHandle.GetResourceIndex()));

    assert(atlasIndex < spriteAtlasTextures_.size());

    return spriteAtlasTextures_[atlasIndex].GetSourceTexture();
}

SpriteDescriptorPackage SpriteAtlas::ExportSpriteDescriptors() const
{
    SpriteDescriptorPackage package;
    /*package.reserve(seriesNameRanges_.size() + size_t{1});

    const size_t numSeriesSprites = std::accumulate(
        seriesNameRanges_.begin(), seriesNameRanges_.end(), size_t{0},
        [](size_t accum, const auto& pair) {
            assert(pair.second.min <= pair.second.max);
            return accum + (pair.second.max - pair.second.min);
        });

    package.emplace_back().data.reserve(spriteInfo_.Size() - numSeriesSprites);

    auto getFreeSpriteDescriptors = [&package] -> SpriteDescriptors& { 
        return package.front(); 
    };
    auto getCurrentSeriesDescriptors = [&package] -> SpriteDescriptors& { 
        return package.back(); 
    };

    auto iter = spriteInfo_.ForEach<&SpriteInfo::spriteName,
                                    &SpriteInfo::seriesName,
                                    &SpriteInfo::filepath>();

    std::string_view currentSeries;

    for (const auto& [spriteName, seriesName, filepath] : iter)
    {
        if (!seriesName.empty() && seriesName != currentSeries)
        {
            auto& newSeries = package.emplace_back();
            newSeries.seriesName = seriesName;

            auto it = seriesNameRanges_.find(seriesName);
            assert(it != seriesNameRanges_.end());
            assert(it->second.min <= it->second.max);

            newSeries.data.reserve(it->second.max - it->second.min);

            currentSeries = seriesName;
        }       

        auto& currentDescriptors = (seriesName.empty())
            ? getFreeSpriteDescriptors()
            : getCurrentSeriesDescriptors();

        currentDescriptors.data.emplace_back(spriteName, filepath);
    }*/

    return package;
}

/** @} */

//void SpriteAtlas::RepopulateSpriteNameIndexMap(size_t newSize)
//{
//    spriteNameIndices_.clear();
//    spriteNameIndices_.reserve(std::max(newSize, spriteNameIndices_.size()));
//
//    for (size_t i = 0; i < spriteInfo_.Size(); ++i)
//    {
//        const auto& name = spriteInfo_.GetView<&SpriteInfo::spriteName>(i);
//
//        auto [_, inserted] = spriteNameIndices_.try_emplace(name, i);
//        assert(inserted);
//    }
//}

//void SpriteAtlas::RepopulateSpriteSeriesRangeMap(size_t newSize)
//{
//    seriesNameRanges_.clear();
//    seriesNameRanges_.reserve(std::max(newSize, seriesNameRanges_.size()));
//
//    std::string_view currentSeries;
//    size_t seriesStartIdx = 0;
//    for (size_t i = 0; i < spriteInfo_.Size(); ++i)
//    {
//        const auto& series = spriteInfo_.GetView<&SpriteInfo::seriesName>(i);
//        if (series != currentSeries)
//        {
//            if (!currentSeries.empty())
//            {
//                assert(i > 0);
//
//                auto [_, inserted] = seriesNameRanges_.try_emplace(
//                    currentSeries, Range<size_t>{seriesStartIdx, i - 1}
//                );
//                assert(inserted);
//            }
//
//            currentSeries = series;
//            seriesStartIdx = i;
//        }
//        else if (series.empty())
//        {
//            seriesStartIdx = i;
//        }
//    }
//
//    if (!currentSeries.empty()) // finished on a series, cap it
//    {
//        auto [_, inserted] = seriesNameRanges_.try_emplace(
//            currentSeries, Range<size_t>{seriesStartIdx, spriteInfo_.Size() - 1}
//        );
//        assert(inserted);
//    }
//}