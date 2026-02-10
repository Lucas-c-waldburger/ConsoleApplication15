#include "SpriteAtlasCollection.h"
#include "../core/Hash.h"
#include "../core/ScopedInvoker.h"
#include "../events/EventBus2.h"
#include <filesystem>
#include <SDL_image.h>

namespace {

constexpr bool IsPowerOfTwo(size_t x) noexcept
{
    return x != 0 && (x & (x - 1)) == 0;
}

} // unnamed

SpriteAtlasTexture::SpriteAtlasTexture(SpriteAtlasTexture&& other) noexcept : 
    TextureAtlas(std::move(other))
{}

SpriteAtlasTexture& SpriteAtlasTexture::operator=(SpriteAtlasTexture&& other) noexcept
{
    if (this != &other)
    {
        TextureAtlas::operator=(std::move(other));
    }
    return *this;
}

Result<SpriteAtlasTexture>
SpriteAtlasTexture::Create(SDL_Renderer* renderer, size_t size)
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

    return atlas;
}

Result<SpriteInfo> SpriteAtlasTexture::LoadSprite(SDL_Renderer* renderer,
											     const SpriteDescriptor& descriptor,
                                                 bool& atlasFull)
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

    spriteSurface = IMG_Load(descriptor.filepath.c_str());
    if (!spriteSurface)
    {
        return MAKE_ERROR(IMG_GetError());
    }

    if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
    {
        return MAKE_ERROR_FMT("Invalid surface dimensions: ({}, {})",
            spriteSurface->w, spriteSurface->h);
    }

    AtlasPlot plot{ .rotation = 0.0f };

    rbp::Rect packed = binPack_.Insert(spriteSurface->w, spriteSurface->h,
        rbp::MaxRectsBinPack::RectBestAreaFit);
    if (!WasRectPacked(packed))
    {
        atlasFull = true;

        return SpriteInfo{};
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

    return SpriteInfo{
        .atlasId = GetAtlasID(),
        .plot = plot
    };
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



Result<Sprite> SpriteAtlas::LoadSprite(SDL_Renderer* renderer,
                                                 SpriteDescriptor&& descriptor)
{
    if (spriteAtlasTextures_.empty())
    {
        assert(textureSize_ > 0);

        TRY_ASSIGN(spriteAtlasTextures_.emplace_back(),
            SpriteAtlasTexture::Create(renderer, textureSize_));

        NotifyTextureCreated(spriteAtlasTextures_.back().GetAtlasID(), 
                             spriteAtlasTextures_.back().GetSourceTexture());
    }
    assert(!spriteAtlasTextures_.empty());

    SDL_SetRenderTarget(renderer, spriteAtlasTextures_.back().GetSourceTexture());

    auto loadResult = LoadSpriteImpl(renderer, std::move(descriptor));

    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

size_t SpriteAtlas::GetTextureCount() const
{
    return spriteAtlasTextures_.size();
}

Result<Sprite> SpriteAtlas::LoadSpriteImpl(SDL_Renderer* renderer,
												     SpriteDescriptor&& descriptor)
{
    if (!std::filesystem::exists(descriptor.filepath))
    {
        return MAKE_ERROR_FMT("Invalid filepath: '{}'", descriptor.filepath);
    }

    if (descriptor.spriteName.empty())
    {
        descriptor.spriteName = 
            std::filesystem::path(descriptor.filepath).stem().string();
    }

    const HashType hashedSpriteName = RapidHash(descriptor.spriteName);
    if (spriteNameIndices_.contains(hashedSpriteName))
    {
        return MAKE_ERROR_FMT("Sprite name '{}' already exists in atlas",
            descriptor.spriteName);
    }

    assert(!spriteAtlasTextures_.empty());

    SpriteInfo newSpriteInfo{};
    bool atlasFull = false;
   
    do 
    {
        atlasFull = false;

        TRY_ASSIGN(newSpriteInfo, spriteAtlasTextures_.back().LoadSprite(
            renderer, descriptor, atlasFull));

        if (atlasFull)
        {
            TRY_ASSIGN(spriteAtlasTextures_.emplace_back(),
                SpriteAtlasTexture::Create(renderer, textureSize_));

            auto& newAtlas = spriteAtlasTextures_.back();

            NotifyTextureCreated(newAtlas.GetAtlasID(), newAtlas.GetSourceTexture());

            SDL_SetRenderTarget(renderer, newAtlas.GetSourceTexture());
        }

    } while (atlasFull);

    newSpriteInfo.spriteName = std::move(descriptor.spriteName);
    newSpriteInfo.filepath = std::move(descriptor.filepath);

    size_t spriteIdx = spriteInfo_.PushBack(std::move(newSpriteInfo));
    spriteNameIndices_.emplace(hashedSpriteName, spriteIdx);

    const auto plot = spriteInfo_.GetView<&SpriteInfo::plot>(spriteIdx);
    assert(plot.rect.w > 0 && plot.rect.h > 0);

    const auto resourceHandle = Handle<TextureResource>::Create(
        spriteAtlasTextures_.back().GetAtlasID(), spriteIdx
    );

    return Sprite{
        .resourceHandle = resourceHandle,
        .plot = plot
    };
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSprites(SDL_Renderer* renderer, SpriteDescriptors&& descriptors)
{
    if (descriptors.data.empty())
    {
        return MAKE_ERROR("Sprite descriptors were empty");
    }

    const bool isSpriteSeries = !descriptors.seriesName.empty();
    if (isSpriteSeries && seriesNameRanges_.contains(descriptors.seriesName))
    {
        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists",
            descriptors.seriesName);
    }

    spriteInfo_.Reserve(spriteInfo_.Size() + descriptors.data.size());

    if (spriteAtlasTextures_.empty())
    {
        assert(textureSize_ > 0);

        TRY_ASSIGN(spriteAtlasTextures_.emplace_back(),
            SpriteAtlasTexture::Create(renderer, textureSize_));

        auto& newAtlas = spriteAtlasTextures_.back();

        NotifyTextureCreated(newAtlas.GetAtlasID(), newAtlas.GetSourceTexture());
    }
    assert(!spriteAtlasTextures_.empty());

    SDL_SetRenderTarget(renderer, spriteAtlasTextures_.back().GetSourceTexture());

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
    auto it = seriesNameRanges_.find(seriesName);
    if (it == seriesNameRanges_.end())
    {
        return {};
    }

    const auto [start, end] = it->second;

    assert(end > start);
    assert(end < spriteInfo_.Size());

    std::vector<Sprite> sprites;
    sprites.reserve(end - start);

    for (size_t i = start; i <= end; i++)
    {
        assert(spriteInfo_.GetView<&SpriteInfo::seriesName>(i) == seriesName);

        sprites.emplace_back(MakeSprite(i));
    }

    return sprites;
}

Sprite SpriteAtlas::GetSpriteSeriesMember(std::string_view seriesName, 
                                                    size_t seriesIdx) const
{
    auto it = seriesNameRanges_.find(seriesName);
    if (it == seriesNameRanges_.end())
    {
        return {};
    }

    const auto [start, end] = it->second;

    assert(end > start);
    assert(end < spriteInfo_.Size());

    size_t adjustedIdx = seriesIdx + start;
    if (adjustedIdx > end)
    {
        LOG_ERROR_FMT("Index '{}' for sprite series '{}' was out of range. "
            "Defaulting to series index 0", seriesIdx, seriesName);

        adjustedIdx = start;
    }

    return MakeSprite(adjustedIdx);
}

size_t SpriteAtlas::GetSpriteSeriesSize(std::string_view seriesName) const
{
    auto it = seriesNameRanges_.find(seriesName);
    if (it == seriesNameRanges_.end())
    {
        return std::numeric_limits<size_t>::max();
    }

    const auto [start, end] = it->second;

    assert(end > start);
    assert(end < spriteInfo_.Size());

    return end - start;
}

bool SpriteAtlas::HasSprite(std::string_view spriteName) const
{
    return spriteNameIndices_.contains(spriteName);
}

bool SpriteAtlas::HasSpriteSeries(std::string_view seriesName) const
{
    return seriesNameRanges_.contains(seriesName);
}

bool SpriteAtlas::IsSpriteValid(const Sprite& sprite) const
{
    const auto spriteIdx = static_cast<size_t>(
        sprite.resourceHandle.GetResourceIndex());

    if (spriteIdx >= spriteInfo_.Size())
    {
        return false;
    }

    const auto [atlasId, plot] = spriteInfo_.GetView<&SpriteInfo::atlasId,
                                                     &SpriteInfo::plot>
                                                     (spriteIdx);

    return sprite.resourceHandle.GetAtlasID() == atlasId && sprite.plot == plot;
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
    const auto [atlasId, plot] = spriteInfo_.GetView<&SpriteInfo::atlasId,
                                                     &SpriteInfo::plot>(spriteIndex);

    return Sprite{
        .resourceHandle = Handle<TextureResource>::Create(atlasId, spriteIndex),
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

    for (size_t i = 0; i < descriptors.data.size(); i++)
    {
        auto& descriptor = descriptors.data[i];

        TRY_ASSIGN(sprites.emplace_back(), 
            LoadSpriteImpl(renderer, std::move(descriptor)));

        if (isSpriteSeries)
        {
            const size_t spriteIdx = 
                static_cast<size_t>(sprites.back().resourceHandle.GetResourceIndex());

            auto [seriesName, seriesIdx] =
                spriteInfo_.GetView<&SpriteInfo::seriesName,
                                    &SpriteInfo::seriesIndex>
                                    (spriteIdx);

            seriesName = descriptors.seriesName;
            seriesIdx = i;
        }
    }

    if (isSpriteSeries)
    {
        const size_t minSpriteIdx = 
            static_cast<size_t>(sprites.front().resourceHandle.GetResourceIndex());
        const size_t maxSpriteIdx = 
            static_cast<size_t>(sprites.back().resourceHandle.GetResourceIndex());

        assert(minSpriteIdx < maxSpriteIdx);

        const auto& seriesNameRef =
            spriteInfo_.GetView<&SpriteInfo::seriesName>(minSpriteIdx);

        seriesNameRanges_.emplace(RapidHash(seriesNameRef), 
            Range<size_t>{ .min = minSpriteIdx, .max = maxSpriteIdx }
        );
    }

    return sprites;
}

SpriteDescriptorPackage SpriteAtlas::ExportSpriteDescriptors() const
{
    SpriteDescriptorPackage package;
    package.reserve(seriesNameRanges_.size() + size_t{1});

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
    }

    return package;
}