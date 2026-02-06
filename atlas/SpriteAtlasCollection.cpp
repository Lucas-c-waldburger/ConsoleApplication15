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

NewSpriteAtlas::NewSpriteAtlas(NewSpriteAtlas&& other) noexcept : 
    TextureAtlas(std::move(other))
{}

NewSpriteAtlas& NewSpriteAtlas::operator=(NewSpriteAtlas&& other) noexcept
{
    if (this != &other)
    {
        TextureAtlas::operator=(std::move(other));
    }
    return *this;
}

Result<NewSpriteAtlas>
NewSpriteAtlas::Create(SDL_Renderer* renderer, size_t size = kDefaultAtlasSize)
{
    NewSpriteAtlas atlas{ Handle<TextureAtlas>::Create() };

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

Result<NewSpriteInfo> NewSpriteAtlas::LoadSprite(SDL_Renderer* renderer,
											     const SpriteDescriptor& descriptor,
                                                 bool& atlasFull)
{
    // info resolved, now load it
    if (!atlasTexture_)
    {
        return MAKE_ERROR("Atlas texture was null");
    }
    assert(GetHandle().IsValid());

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

        return NewSpriteInfo{};
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

    return NewSpriteInfo{
        .textureHandle = GetHandle(),
        .plot = plot
    };
}

Result<Void> NewSpriteAtlas::RebuildSourceTexture(SDL_Renderer* renderer, 
                                                  const NewSpriteInfoSOA& spriteInfo, 
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
            spriteInfo.GetView<&NewSpriteInfo::textureHandle>(runningIdxCounter) ==
            GetHandle();
    };

    while (run())
    {
        const auto& [plot, path] = spriteInfo.GetView<&NewSpriteInfo::plot,
                                                      &NewSpriteInfo::filepath>
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



Result<Sprite> SpriteAtlasCollection::LoadSprite(SDL_Renderer* renderer,
                                                     SpriteDescriptor&& descriptor)
{
    if (spriteAtlases_.empty())
    {
        assert(textureSize_ > 0);

        TRY_ASSIGN(spriteAtlases_.emplace_back(),
            NewSpriteAtlas::Create(renderer, textureSize_));
    }
    assert(!spriteAtlases_.empty());

    SDL_SetRenderTarget(renderer, spriteAtlases_.back().GetSourceTexture());

    auto loadResult = LoadSpriteImpl(renderer, std::move(descriptor));

    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}


void SpriteAtlasCollection::ConnectTextureRebuildSignal(SDL_Renderer* renderer, 
                                                        EventBus2& bus)
{
    if (!rebuildTexturesSignalToken_.IsConnected())
    {
        rebuildTexturesSignalToken_ = bus.ConnectToEvent(
            [renderer, this](const events::RenderReset&) {
                LOG_IF_ERROR(this->RebuildSourceTextures(renderer));
            });
    }
}

Result<Sprite> SpriteAtlasCollection::LoadSpriteImpl(SDL_Renderer* renderer,
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

    const size_t hashedSpriteName = RapidHash(descriptor.spriteName);
    if (spriteNameIndices_.contains(hashedSpriteName))
    {
        return MAKE_ERROR_FMT("Sprite name '{}' already exists in atlas",
            descriptor.spriteName);
    }

    assert(!spriteAtlases_.empty());

    NewSpriteInfo newSpriteInfo{};
    bool atlasFull = false;
   
    do 
    {
        atlasFull = false;

        TRY_ASSIGN(newSpriteInfo, spriteAtlases_.back().LoadSprite(
            renderer, descriptor, atlasFull));

        if (atlasFull)
        {
            TRY_ASSIGN(spriteAtlases_.emplace_back(),
                NewSpriteAtlas::Create(renderer, textureSize_));

            SDL_SetRenderTarget(renderer, spriteAtlases_.back().GetSourceTexture());
        }

    } while (atlasFull);

    newSpriteInfo.spriteName = std::move(descriptor.spriteName);
    newSpriteInfo.filepath = std::move(descriptor.filepath);

    size_t spriteIdx = spriteInfo_.PushBack(std::move(newSpriteInfo));
    spriteNameIndices_.emplace(hashedSpriteName, spriteIdx);

    const auto& plot = spriteInfo_.GetView<&NewSpriteInfo::plot>(spriteIdx);
    assert(plot.rect.w > 0 && plot.rect.h > 0);

    return Sprite{
        .sourceAtlas = spriteAtlases_.back().GetHandle(),
        .plot = plot,
        .spriteIndex = spriteIdx
    };
}

Result<std::vector<Sprite>> 
SpriteAtlasCollection::LoadSprites(SDL_Renderer* renderer, SpriteDescriptors&& descriptors)
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

    if (spriteAtlases_.empty())
    {
        assert(textureSize_ > 0);

        TRY_ASSIGN(spriteAtlases_.emplace_back(),
            NewSpriteAtlas::Create(renderer, textureSize_));
    }
    assert(!spriteAtlases_.empty());

    SDL_SetRenderTarget(renderer, spriteAtlases_.back().GetSourceTexture());

    auto loadResult = LoadSpritesImpl(renderer, std::move(descriptors));

    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Sprite SpriteAtlasCollection::GetSprite(std::string_view spriteName)
{
    auto it = spriteNameIndices_.find(spriteName);
    if (it == spriteNameIndices_.end())
    {
        return {};
    }

    assert(it->second < spriteInfo_.Size());
    assert(spriteInfo_.GetView<&NewSpriteInfo::spriteName>(it->second) == spriteName);

    return MakeSprite(it->second);
}

std::vector<Sprite> SpriteAtlasCollection::GetSpriteSeries(std::string_view seriesName)
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
        assert(spriteInfo_.GetView<&NewSpriteInfo::seriesName>(i) == seriesName);

        sprites.emplace_back(MakeSprite(i));
    }

    return sprites;
}

Sprite SpriteAtlasCollection::GetSpriteSeriesMember(std::string_view seriesName, 
                                                    size_t seriesIdx)
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

size_t SpriteAtlasCollection::GetSpriteSeriesSize(std::string_view seriesName) const
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

bool SpriteAtlasCollection::HasSprite(std::string_view spriteName) const
{
    return spriteNameIndices_.contains(spriteName);
}

bool SpriteAtlasCollection::HasSpriteSeries(std::string_view seriesName) const
{
    return seriesNameRanges_.contains(seriesName);
}

bool SpriteAtlasCollection::IsSpriteValid(const Sprite& sprite) const
{
    if (sprite.spriteIndex >= spriteInfo_.Size())
    {
        return false;
    }

    const auto& [handle, plot] = spriteInfo_.GetView<&NewSpriteInfo::textureHandle,
                                                     &NewSpriteInfo::plot>
                                                     (sprite.spriteIndex);

    return sprite.sourceAtlas == handle && sprite.plot == plot;
}

Result<Void> SpriteAtlasCollection::RebuildSourceTextures(SDL_Renderer* renderer)
{
    size_t runningIdxCounter = 0;

    for (auto& spriteAtlas : spriteAtlases_)
    {
        TRY(spriteAtlas.RebuildSourceTexture(renderer, spriteInfo_, runningIdxCounter));
    }

    assert(runningIdxCounter == spriteInfo_.Size());

    return kVoid;
}

auto SpriteAtlasCollection::GetSpriteInfo(const Sprite& sprite) const
{
    using Ret = decltype(spriteInfo_.TryGetView(0));

    if (!IsSpriteValid(sprite))
    {
        return Ret{ std::nullopt };
    }

    const auto& cInfo = spriteInfo_;
    return cInfo.TryGetView(sprite.spriteIndex);
}

Sprite SpriteAtlasCollection::MakeSprite(size_t spriteIndex) const
{
    auto [handle, plot] = spriteInfo_.GetView<&NewSpriteInfo::textureHandle,
                                              &NewSpriteInfo::plot>(spriteIndex);

    return Sprite{
        .sourceAtlas = handle,
        .plot = plot,
        .spriteIndex = spriteIndex
    };
}

Result<std::vector<Sprite>>
SpriteAtlasCollection::LoadSpritesImpl(SDL_Renderer* renderer,
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
            auto [seriesName, seriesIdx] =
                spriteInfo_.GetView<&NewSpriteInfo::seriesName,
                                    &NewSpriteInfo::seriesIndex>
                                    (sprites.back().spriteIndex);

            seriesName = descriptors.seriesName;
            seriesIdx = i;
        }
    }

    if (isSpriteSeries)
    {
        const size_t minSpriteIdx = sprites.front().spriteIndex;
        const size_t maxSpriteIdx = sprites.back().spriteIndex;

        assert(minSpriteIdx < maxSpriteIdx);

        const auto& seriesNameRef =
            spriteInfo_.GetView<&SpriteInfo::seriesName>(minSpriteIdx);

        seriesNameRanges_.emplace(RapidHash(seriesNameRef), 
            Range<size_t>{ .min = minSpriteIdx, .max = maxSpriteIdx }
        );
    }

    return sprites;
}