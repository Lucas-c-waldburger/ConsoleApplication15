#include "SpriteAtlas.h"
#include <SDL_image.h>
#include <cassert>
#include <ranges>
#include <filesystem>
#include "PackingTools.h"
#include "../core/Algorithms.h"
#include "../core/Handle.h"
#include "../core/commonObjects.h"
#include "../core/ScopedInvoker.h"

namespace {

//struct SpriteSeriesLayoutHandler
//{
//    static constexpr std::string_view kInconsistencyErrFmt =
//        "Series '{}' index inconsistency. Expected index: {}, Actual index: {}";
//
//    enum : uint8_t
//    {
//        NewSeries = 1 << 0,
//        SameSeries = 1 << 1,
//        EndOfSeries = 1 << 2,
//        Pass = 1 << 3
//    };
//
//    uint8_t GetStatus(std::string_view seriesName, size_t seriesIdx) const noexcept 
//    {
//        uint8_t result = 0;
//
//        if (!seriesName.empty())
//        {
//            if (seriesName == currentSeries)
//            {
//                return SameSeries;
//            }
//
//
//        }
//
//        if (seriesName.empty())
//        {
//            if (currentSeries.empty())
//            {
//                return Pass;
//            }
//
//            return EndOfSeries;
//        }
//
//        if (currentSeries.empty())
//    }
//
//    explicit SpriteSeriesLayoutHandler(SpriteAtlas::SeriesRangeMap& rangeMap) :
//        seriesRanges(rangeMap) {}
//    ~SpriteSeriesLayoutHandler() = default;
//
//    SpriteSeriesLayoutHandler(const SpriteSeriesLayoutHandler&) = delete;
//    SpriteSeriesLayoutHandler& operator=(const SpriteSeriesLayoutHandler&) = delete;
//
//    SpriteSeriesLayoutHandler(SpriteSeriesLayoutHandler&&) = delete;
//    SpriteSeriesLayoutHandler& operator=(SpriteSeriesLayoutHandler&&) = delete;
//
//    Result<Void> VerifyAndUpdate(std::string_view seriesName, size_t seriesIdx)
//    {
//        if (!seriesName.empty())
//        {
//            if (seriesName != currentSeries)
//            {
//                // new series
//                if (seriesIdx != 0)
//                {
//                    return MAKE_ERROR_FMT(kInconsistencyErrFmt, 0, seriesIdx);
//                }
//            }
//            else if (seriesIdx != seriesIdxCounter + 1)
//            {
//                return MAKE_ERROR_FMT(kInconsistencyErrFmt, (seriesIdxCounter + 1), 
//                    seriesIdx);
//            }
//        }
//        else if ()
//
//        currentSeries = seriesName;
//        seriesIdxCounter = seriesIdx;
//
//        return Void{};
//    }
//
//    SpriteAtlas::SeriesRangeMap& seriesRanges;
//    size_t seriesIdxCounter = 0;
//    std::string_view currentSeries;
//};

bool ValidSurfacePlot(SDL_Surface* surface, const AtlasPlot& plot)
{
    return surface && 
           surface->w == plot.rect.w && surface->h == plot.rect.h &&
           plot.rect.w > 0 && plot.rect.h;
}


constexpr bool IsPowerOfTwo(size_t x) noexcept 
{
    return x != 0 && (x & (x - 1)) == 0;
}

} // unnamed namespace

/* BASE SPRITE ATLAS */

//SpriteInfo BaseSpriteAtlas::GetSpriteInfo(const Sprite& sprite) const
//{
//    auto validated = ValidateSprite(sprite);
//    if (!validated.Success())
//    {
//        LOG_ERROR(validated.GetError());
//        return {};
//    }
//
//    return spriteInfo_.MakeSlice(sprite.spriteIndex);
//}
//
//Result<Void> BaseSpriteAtlas::ValidateSprite(const Sprite& sprite) const
//{
//    if (sprite.sourceAtlas != GetHandle())
//    {
//        return MAKE_ERROR("Sprite does not belong to this atlas");
//    }
//    if (sprite.spriteIndex >= spriteInfo_.Size())
//    {
//        return MAKE_ERROR_FMT("Invalid sprite index '{}'", sprite.spriteIndex);
//    }
//    if (sprite.plot.rect.w <= 0 && sprite.plot.rect.h <= 0)
//    {
//        return MAKE_ERROR_FMT("Sprite atlas plot dimensions invalid: ({}, {})",
//            sprite.plot.rect.w, sprite.plot.rect.h);
//    }
//
//    return Void{};
//}
//
//bool BaseSpriteAtlas::IsSpriteValid(const Sprite& sprite) const
//{
//    return sprite.sourceAtlas == GetHandle() &&
//           sprite.spriteIndex < spriteInfo_.Size() &&
//           sprite.plot.rect.w > 0 && sprite.plot.rect.h > 0;
//}
//
//Sprite BaseSpriteAtlas::GetSprite(std::string_view spriteName) const
//{
//    size_t i = 0;
//    for (const auto& name : spriteInfo_.ForEach<&SpriteInfo::spriteName>())
//    {
//        if (name == spriteName)
//        {
//            assert(i < spriteInfo_.Size());
//            return MakeSprite(i);
//        }
//
//        ++i;
//    }
//
//    return {};
//}
//
//std::vector<Sprite> BaseSpriteAtlas::GetSpriteSeries(std::string_view seriesName) const
//{
//    auto it = spriteSeriesRanges_.find(seriesName);
//    if (it == spriteSeriesRanges_.end())
//    {
//        return {};
//    }
//
//    const auto [start, end] = it->second;
//
//    assert(end > start);
//    assert(end < spriteInfo_.Size());
//
//    std::vector<Sprite> sprites;
//    sprites.reserve(end - start);
//
//    for (size_t i = start; i <= end; i++)
//    {
//        sprites.emplace_back(MakeSprite(i));
//    }
//
//    return sprites;
//}
//
//Sprite BaseSpriteAtlas::MakeSprite(size_t spriteIndex) const
//{
//    if (spriteIndex >= spriteInfo_.Size())
//    {
//        return {};
//    }
//
//    const auto& plot = spriteInfo_.GetView<&SpriteInfo::plot>(spriteIndex);
//
//    return Sprite{
//        .sourceAtlas = GetHandle(),
//        .plot = plot,
//        .spriteIndex = spriteIndex
//    };
//}
//
///* --- */
//
//
///* DYNAMIC SPRITE ATLAS */
//
//Result<DynamicSpriteAtlas> DynamicSpriteAtlas::Create(SDL_Renderer* renderer, size_t size)
//{
//    DynamicSpriteAtlas atlas{ Handle<TextureAtlas>::Create() };
//
//    atlas.textureSize_ = IsPowerOfTwo(size) ? size :
//        size > kMaxAtlasSize ? kMaxAtlasSize :
//        GetNextPowerOfTwo(static_cast<int>(size));
//
//    atlas.atlasTexture_ = MakeUniqueTexturePtr(
//        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size
//    );
//    if (!atlas.atlasTexture_)
//    {
//        return MAKE_ERROR(SDL_GetError());
//    }
//
//    SDL_SetTextureBlendMode(atlas.atlasTexture_.get(), SDL_BLENDMODE_BLEND);
//
//    atlas.binPack_.Init(size, size, false);
//
//    return atlas;
//}
//
//Result<Sprite>
//DynamicSpriteAtlas::LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
//{
//    SDL_SetRenderTarget(renderer, atlasTexture_.get());
//    auto loadResult = LoadSpriteImpl(renderer, std::move(descriptor));
//    SDL_SetRenderTarget(renderer, nullptr);
//
//    return loadResult;
//}
//
//Result<std::vector<Sprite>>
//DynamicSpriteAtlas::LoadSprites(SDL_Renderer* renderer, SpriteDescriptorPackage&& package)
//{
//    SDL_SetRenderTarget(renderer, atlasTexture_.get());
//    auto loadResult = LoadSpritesImpl(renderer, std::move(package));
//    SDL_SetRenderTarget(renderer, nullptr);
//
//    return loadResult;
//}
//
//Result<std::vector<Sprite>>
//DynamicSpriteAtlas::LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptorPackage&& package)
//{
//    if (package.descriptors.empty())
//    {
//        return MAKE_ERROR("Sprite descriptor package was empty");
//    }
//
//    const bool isSpriteSeries = !package.seriesName.empty();
//    if (isSpriteSeries && spriteSeriesRanges_.contains(package.seriesName))
//    {
//        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists",
//            package.seriesName);
//    }
//
//    spriteInfo_.Reserve(spriteInfo_.Size() +
//        package.descriptors.size());
//
//    std::vector<Sprite> sprites;
//    sprites.reserve(package.descriptors.size());
//
//    SDL_SetRenderTarget(renderer, atlasTexture_.get());
//
//    for (size_t i = 0; i < package.descriptors.size(); i++)
//    {
//        auto& newSprite = sprites.emplace_back();
//        TRY_ASSIGN(newSprite, LoadSpriteImpl(renderer,
//            std::move(package.descriptors[i])));
//
//        if (isSpriteSeries)
//        {
//            auto [seriesName, seriesIdx] =
//                spriteInfo_.GetView<&SpriteInfo::seriesName,
//                &SpriteInfo::seriesIndex>(newSprite.spriteIndex);
//
//            seriesName = package.seriesName;
//            seriesIdx = i;
//        }
//    }
//
//    if (isSpriteSeries)
//    {
//        assert(sprites.front().spriteIndex < sprites.back().spriteIndex);
//
//        spriteSeriesRanges_[std::move(package.seriesName)] = {
//            .min = static_cast<size_t>(sprites.front().spriteIndex),
//            .max = static_cast<size_t>(sprites.back().spriteIndex)
//        };
//    }
//
//    return sprites;
//}
//
//Result<Sprite> 
//DynamicSpriteAtlas::LoadSpriteImpl(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
//{
//    if (!atlasTexture_)
//    {
//        return MAKE_ERROR("Atlas texture was null");
//    }
//    assert(GetHandle().IsValid());
//
//    SDL_Surface* spriteSurface = nullptr;
//    SDL_Texture* spriteTexture = nullptr;
//
//    ScopedInvoker freeResources{ [&] {
//        SDL_FreeSurface(spriteSurface);
//        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
//    } };
//
//    spriteSurface = IMG_Load(descriptor.filepath.c_str());
//    if (!spriteSurface)
//    {
//        return MAKE_ERROR(IMG_GetError());
//    }
//
//    if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
//    {
//        return MAKE_ERROR_FMT("Invalid surface dimensions: ({}, {})",
//            spriteSurface->w, spriteSurface->h);
//    }
//
//    AtlasPlot plot{ .rotation = 0.0f };
//
//    rbp::Rect packed = binPack_.Insert(spriteSurface->w, spriteSurface->h,
//        rbp::MaxRectsBinPack::RectBestAreaFit);
//    if (!WasRectPacked(packed))
//    {
//        return MAKE_ERROR("Sprite atlas is full");
//    }
//
//    plot.rect = RbpToSDLRect(packed);
//
//    spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurface);
//    if (!spriteTexture)
//    {
//        return MAKE_ERROR(SDL_GetError());
//    }
//
//    if (SDL_RenderCopy(renderer, spriteTexture, nullptr, &plot.rect) != 0)
//    {
//        return MAKE_ERROR(SDL_GetError());
//    }
//
//    if (descriptor.spriteName.empty())
//    {
//        descriptor.spriteName =
//            std::filesystem::path(descriptor.filepath).stem().string();
//    }
//
//    size_t spriteIdx = spriteInfo_.PushBack({
//        .plot = plot,
//        .spriteName = std::move(descriptor.spriteName),
//        .filepath = std::move(descriptor.filepath)
//        });
//
//
//    return MakeSprite(spriteIdx);
//}
//
///* --- */
//
//
///* FIXED SPRITE ATLAS */
//
//Result<FixedSpriteAtlas>
//FixedSpriteAtlas::Create(SDL_Renderer* renderer, SpriteInfoSOA&& spriteInfo, 
//                         UnorderedDictionary<Range<size_t>>&& seriesRanges,
//                         size_t origAtlasSize)
//{
//    if (spriteInfo.Empty())
//    {
//        return MAKE_ERROR("Sprite Info was empty");
//    }
//
//    FixedSpriteAtlas atlas{ Handle<TextureAtlas>::Create() };
//
//    atlas.spriteInfo_ = std::move(spriteInfo);
//    atlas.spriteSeriesRanges_ = std::move(seriesRanges);
//
//    // load atlas texture from original size
//    assert(origAtlasSize > 0);
//
//    atlas.atlasTexture_ = MakeUniqueTexturePtr(
//        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
//        origAtlasSize, origAtlasSize
//    );
//    if (!atlas.atlasTexture_)
//    {
//        return MAKE_ERROR(SDL_GetError());
//    }
//
//    SDL_SetTextureBlendMode(atlas.atlasTexture_.get(), SDL_BLENDMODE_BLEND);
//
//    SDL_Surface* spriteSurface = nullptr;
//    SDL_Texture* spriteTexture = nullptr;
//
//    ScopedInvoker freeResources{ [&] {
//        SDL_FreeSurface(spriteSurface);
//        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
//    } };
//
//    // load sprites from existing info, validate 
//    auto iter = atlas.spriteInfo_.ForEach<&SpriteInfo::plot, 
//                                          &SpriteInfo::filepath,
//                                          &SpriteInfo::seriesName, 
//                                          &SpriteInfo::seriesIndex>();
//
//    SpriteInfoLayoutValidator layoutValidator{ atlas.spriteSeriesRanges_ };
//
//    for (auto [plot, path, seriesName, seriesIdx] : iter)
//    {
//        TRY(layoutValidator.Validate(seriesName, seriesIdx));
//
//        spriteSurface = IMG_Load(path.c_str());
//        if (!spriteSurface)
//        {
//            return MAKE_ERROR(IMG_GetError());
//        }
//
//        if (!ValidSurfacePlot(spriteSurface, plot))
//        {
//            return MAKE_ERROR_FMT("Surface and plot sizes did not match. "
//                "surface dimensions: ({}, {}), plot dimensions: ({}, {})",
//                spriteSurface->w, spriteSurface->h, plot.rect.w, plot.rect.h);
//        }
//
//        spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurface);
//        if (!spriteTexture)
//        {
//            return MAKE_ERROR(SDL_GetError());
//        }
//
//        if (SDL_RenderCopy(renderer, spriteTexture, nullptr, &plot.rect) != 0)
//        {
//            return MAKE_ERROR(SDL_GetError());
//        }
//    }
//
//    return atlas;
//}


/* --- */


Result<Sprite> 
SpriteAtlas::LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
{
    SDL_SetRenderTarget(renderer, atlasTexture_.get());
    auto loadResult = LoadSpriteImpl(renderer, std::move(descriptor));
    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSprites(SDL_Renderer* renderer, SpriteDescriptors&& package)
{
    SDL_SetRenderTarget(renderer, atlasTexture_.get());
    auto loadResult = LoadSpritesImpl(renderer, std::move(package));
    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptors&& package)
{
    if (package.data.empty())
    {
        return MAKE_ERROR("Sprite descriptor package was empty");
    }

    const bool isSpriteSeries = !package.seriesName.empty();
    if (isSpriteSeries && seriesRanges_.contains(package.seriesName))
    {
        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists",
            package.seriesName);
    }

    spriteInfo_.Reserve(spriteInfo_.Size() +
                        package.data.size());

    std::vector<Sprite> sprites;
    sprites.reserve(package.data.size());

    SDL_SetRenderTarget(renderer, atlasTexture_.get());

    for (size_t i = 0; i < package.data.size(); i++)
    {
        auto& newSprite = sprites.emplace_back();
        TRY_ASSIGN(newSprite, LoadSpriteImpl(renderer, 
            std::move(package.data[i])));

        if (isSpriteSeries)
        {
            auto [seriesName, seriesIdx] = 
                spriteInfo_.GetView<&SpriteInfo::seriesName,
                                    &SpriteInfo::seriesIndex>(newSprite.spriteIndex);

            seriesName = package.seriesName;
            seriesIdx = i;
        }
    }
     
    if (isSpriteSeries)
    { 
        assert(sprites.front().spriteIndex < sprites.back().spriteIndex);

        seriesRanges_[std::move(package.seriesName)] = {
            .min = static_cast<size_t>(sprites.front().spriteIndex),
            .max = static_cast<size_t>(sprites.back().spriteIndex)
        };
    }

    return sprites;
}

Result<SpriteAtlas> SpriteAtlas::Create(SDL_Renderer* renderer, size_t size)
{
    SpriteAtlas atlas{ Handle<TextureAtlas>::Create() };

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

Result<SpriteAtlas> SpriteAtlas::Create(SDL_Renderer* renderer, 
                                        SpriteInfoSOA&& spriteInfo, size_t size)
{
    if (spriteInfo.Size() == 0)
    {
        return MAKE_ERROR("Sprite info was empty");
    }

    TRY(Create(renderer, size), atlas);

    atlas.spriteInfo_ = std::move(spriteInfo);

    // setup render resources
    SDL_SetRenderTarget(renderer, atlas.atlasTexture_.get());

    SDL_Surface* spriteSurface = nullptr;
    SDL_Texture* spriteTexture = nullptr;

    ScopedInvoker freeResources{ [&] {
        SDL_FreeSurface(spriteSurface);
        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
        SDL_SetRenderTarget(renderer, nullptr);
    } };

    // setup index tracking
    size_t spriteIdx = 0;
    size_t lastSeriesIdx = 0;
    std::string_view currentSeries;

    // do the thing
    for (auto [plot, name, path, series, seriesIdx] : atlas.spriteInfo_.ForEach())
    {
        // validate sprite name
        if (name.empty())
        {
            return MAKE_ERROR("Sprite name cannot be empty when "
                "creating sprite atlas with sprite info");
        }
        if (atlas.spriteIndices_.contains(name))
        {
            return MAKE_ERROR_FMT("Duplicate sprite name '{}' in atlas", name);
        }
        
        // validate filepath
        if (path.empty())
        {
            return MAKE_ERROR("Sprite filepath was empty");
        }
        if (!std::filesystem::exists(path))
        {
            return MAKE_ERROR_FMT("Invalid filepath: '{}'", path);
        }

        spriteSurface = IMG_Load(path.c_str());
        if (!spriteSurface)
        {
            return MAKE_ERROR(IMG_GetError());
        }

        // validate dimensions
        if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
        {
            return MAKE_ERROR_FMT("Invalid surface dimensions: ({}, {})",
                spriteSurface->w, spriteSurface->h);
        }
        if (spriteSurface->w != plot.rect.w || spriteSurface->h != plot.rect.h)
        {
            LOG_WARNING_FMT("Sprite surface dimensions: ({}, {}) did not match "
                "Sprite info plot dimensions: ({}, {})", spriteSurface->w, 
                spriteSurface->h, plot.rect.w, plot.rect.h);
        }

        rbp::Rect packed = atlas.binPack_.Insert(spriteSurface->w, spriteSurface->h,
                                                 rbp::MaxRectsBinPack::RectBestAreaFit);
        if (!WasRectPacked(packed))
        {
            return MAKE_ERROR("Sprite atlas size could not fit all sprite info plots");
        }

        plot.rect = RbpToSDLRect(packed);

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

        // handle series data
        const bool startNewSeries = !series.empty() && 
                                     series != currentSeries;
        const bool endLastSeries = !currentSeries.empty() && 
                                    series != currentSeries;
        const bool stillInSeries = !currentSeries.empty() && 
                                    series == currentSeries;

        if (startNewSeries || endLastSeries)
        {
            if (startNewSeries)
            {
                if (atlas.seriesRanges_.contains(series))
                {
                    return MAKE_ERROR_FMT("Duplicate series name or series index "
                        "inconsistency: '{}'", series);
                }
                if (seriesIdx != 0)
                {
                    return MAKE_ERROR_FMT("Series '{}' index inconsistency. "
                        "Expected index : {}, Actual index : {}",
                        series, 0, seriesIdx);
                }

                atlas.seriesRanges_[series].min = spriteIdx;
            }
            if (endLastSeries)
            {
                atlas.seriesRanges_[currentSeries].max = spriteIdx - 1;
            }
        }
        else if (stillInSeries && seriesIdx != lastSeriesIdx + 1)
        {
            return MAKE_ERROR_FMT("Series '{}' index inconsistency. "
                "Expected index : {}, Actual index : {}",
                series, lastSeriesIdx + 1, seriesIdx);
        }

        atlas.spriteIndices_[name] = spriteIdx;

        // update trackers
        ++spriteIdx;
        currentSeries = series;
        lastSeriesIdx = seriesIdx;
    }

    auto& lastSeries = atlas.spriteInfo_.GetView<&SpriteInfo::seriesName>(
                                         atlas.spriteInfo_.Size() - 1);
    if (!lastSeries.empty())
    {
        atlas.seriesRanges_[lastSeries].max = atlas.spriteInfo_.Size() - 1;
    }

    return atlas;
}

//Result<SpriteAtlas> 
//SpriteAtlas::CreateFromSpriteInfo(SDL_Renderer* renderer,
//                                  SpriteInfoSOA&& spriteInfo, size_t size)
//{
//    if (spriteInfo.Empty())
//    {
//        return MAKE_ERROR("Sprite descriptor package was empty");
//    }
//
//    TRY(SpriteAtlas::Create(renderer, size), atlas);
//
//    atlas.spriteInfo_ = std::move(spriteInfo);
//
//    std::pair<SpriteDescriptorPackage, std::vector<SpriteDescriptorPackage>>
//    tempPackageStage{};
//
//    auto& mainPackage = tempPackageStage.first;
//
//    //packages.reserve(atlas.spriteInfo_.Size());
//
//    for (auto [plot, path, series] : atlas.spriteInfo_.ForEach<&SpriteInfo::plot, 
//                                                               &SpriteInfo::filepath,
//                                                               &SpriteInfo::seriesName>())
//    {
//        if (!series.empty())
//        {
//
//        }
//    }
//}

//SpriteInfo SpriteAtlas::GetSpriteInfo(const Sprite& sprite) const
//{
//    auto validated = ValidateSprite(sprite);
//    if (!validated.Success())
//    {
//        LOG_ERROR(validated.GetError());
//        return {};
//    }
//    
//    return spriteInfo_.MakeSlice(sprite.spriteIndex);
//}

Result<Void> SpriteAtlas::ValidateSprite(const Sprite& sprite) const
{
    if (sprite.sourceAtlas != GetHandle())
    {
        return MAKE_ERROR("Sprite does not belong to this atlas");
    }
    if (sprite.spriteIndex >= spriteInfo_.Size())
    {
        return MAKE_ERROR_FMT("Sprite index '{}' out of range", sprite.spriteIndex);
    }
    if (sprite.plot.rect.w <= 0 && sprite.plot.rect.h <= 0)
    {
        return MAKE_ERROR_FMT("Sprite atlas plot dimensions invalid: ({}, {})",
            sprite.plot.rect.w, sprite.plot.rect.h);
    }

    return Void{};
}

bool SpriteAtlas::IsSpriteValid(const Sprite& sprite) const
{
    return sprite.sourceAtlas == GetHandle() &&
           sprite.spriteIndex < spriteInfo_.Size() &&
           sprite.plot.rect.w > 0 && sprite.plot.rect.h > 0;
}

//bool SpriteAtlas::CanFitSprite(SDL_Renderer* renderer, const SpriteDescriptor& descriptor) const
//{
//    SDL_Surface* spriteSurface = IMG_Load(descriptor.filepath.c_str());
//    if (!spriteSurface)
//    {
//        return false;
//    }
//
//    Dimensions<int> spriteDims = { spriteSurface->w, spriteSurface->h }; 
//
//    SDL_FreeSurface(spriteSurface);
//
//    if (spriteDims.w <= 0 || spriteDims.h <= 0)
//    {
//        return false;
//    }
//
//    return binPack_.WillFit(spriteDims.w, spriteDims.h,
//                            rbp::MaxRectsBinPack::RectBestAreaFit);
//}

Sprite SpriteAtlas::GetSprite(std::string_view spriteName) const
{
    auto it = spriteIndices_.find(spriteName);
    if (it == spriteIndices_.end())
    {
        return {};
    }

    assert(it->second < spriteInfo_.Size());

    return MakeSprite(it->second);
}

std::vector<Sprite> SpriteAtlas::GetSpriteSeries(std::string_view seriesName) const
{
    auto it = seriesRanges_.find(seriesName);
    if (it == seriesRanges_.end())
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
        sprites.emplace_back(MakeSprite(i));
    }

    return sprites;
}

Sprite SpriteAtlas::MakeSprite(size_t spriteIndex) const
{
    if (spriteIndex >= spriteInfo_.Size())
    {
        return {};
    }

    const auto& plot = spriteInfo_.GetView<&SpriteInfo::plot>(spriteIndex);

    return Sprite{
        .sourceAtlas = GetHandle(),
        .plot = plot,
        .spriteIndex = spriteIndex
    };
}

//Result<Sprite> SpriteAtlas::LoadSpriteImpl(SDL_Renderer* renderer,
//                                           SpriteDescriptor&& descriptor)
//{
//    if (!atlasTexture_)
//    {
//        return MAKE_ERROR("Atlas texture was null");
//    }
//    assert(GetHandle().IsValid());
//
//    SDL_Surface* spriteSurface = nullptr;
//    SDL_Texture* spriteTexture = nullptr;
//
//    ScopedInvoker freeResources{ [&] { 
//        SDL_FreeSurface(spriteSurface);
//        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
//    }};
//    
//    spriteSurface = IMG_Load(descriptor.filepath.c_str());
//    if (!spriteSurface)
//    {
//        return MAKE_ERROR(IMG_GetError());
//    }
//
//    if (spriteSurface->w <= 0 || spriteSurface->h <= 0)
//    {
//        return MAKE_ERROR_FMT("Invalid surface dimensions: ({}, {})", 
//                              spriteSurface->w, spriteSurface->h);
//    }
//
//    AtlasPlot plot{ .rotation = 0.0f };
//
//    rbp::Rect packed = binPack_.Insert(spriteSurface->w, spriteSurface->h,
//                                       rbp::MaxRectsBinPack::RectBestAreaFit);
//    if (!WasRectPacked(packed))
//    {
//        return MAKE_ERROR("Sprite atlas is full");
//    }
//
//    plot.rect = RbpToSDLRect(packed);
//
//    spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurface);
//    if (!spriteTexture)
//    {
//        return MAKE_ERROR(SDL_GetError());
//    }
//
//    if (SDL_RenderCopy(renderer, spriteTexture, nullptr, &plot.rect) != 0)
//    {
//        return MAKE_ERROR(SDL_GetError());
//    }
//
//    if (descriptor.spriteName.empty())
//    {
//        descriptor.spriteName = 
//            std::filesystem::path(descriptor.filepath).stem().string();
//    }
//
//    size_t spriteIdx = spriteInfo_.PushBack({
//        .plot = plot,
//        .spriteName = std::move(descriptor.spriteName),
//        .filepath = std::move(descriptor.filepath)
//    });
//
//
//    return MakeSprite(spriteIdx);
//}

Result<Sprite> SpriteAtlas::LoadSpriteImpl(SDL_Renderer* renderer, 
                                           SpriteDescriptor&& descriptor)
{
    if (descriptor.filepath.empty())
    {
        return MAKE_ERROR("Sprite filepath was empty");
    }
    if (!std::filesystem::exists(descriptor.filepath))
    {
        return MAKE_ERROR_FMT("Invalid filepath: '{}'", descriptor.filepath);
    }

    if (descriptor.spriteName.empty())
    {
        descriptor.spriteName = std::filesystem::path(descriptor.filepath).stem().string();
    }
    if (spriteIndices_.contains(descriptor.spriteName))
    {
        // either return the loaded sprite or error if trying to reassign name to different png
        size_t spriteIdx = spriteIndices_[descriptor.spriteName];
        const auto& existingPath = spriteInfo_.GetView<&SpriteInfo::filepath>(spriteIdx);

        if (descriptor.filepath != existingPath)
        {
            return MAKE_ERROR("Sprite name '{}' already exists for a different filepath",
                spriteName);
        }

        return MakeSprite(spriteIdx);
    }

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
        return MAKE_ERROR("Sprite atlas is full");
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

    size_t spriteIdx = spriteInfo_.PushBack({
        .plot = plot,
        .spriteName = std::move(descriptor.spriteName),
        .filepath = std::move(descriptor.filepath)
    });

    auto& nameRef = spriteInfo_.GetView<&SpriteInfo::spriteName>(spriteIdx); 
    spriteIndices_[nameRef] = spriteIdx;

    return Sprite{
        .sourceAtlas = GetHandle(),
        .plot = plot,
        .spriteIndex = spriteIdx
    };
}

