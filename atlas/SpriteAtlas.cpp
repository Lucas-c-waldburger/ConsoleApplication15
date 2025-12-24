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
#include "../serial/SerializationUtils.h"
#include "../serial/user_types/AtlasJsonUserTypes.h"

namespace {

bool SameSeriesName(const auto& lhs, const auto& rhs)
{
    const auto& [lhsSeries, _] = lhs;
    const auto& [rhsSeries, _] = rhs;

    return lhsSeries == rhsSeries;
}

//struct SpriteInfoLoadHelper
//{
//    size_t spriteIndex = 0;
//    std::string_view runningSeriesName;
//    size_t runningSeriesIndex = 0;
//
//    Result<Void> FillMaps(const SpriteInfoSOA& spriteInfo, 
//                          SpriteAtlas::SpriteIndexMap& spriteIndices,
//                          SpriteAtlas::SeriesRangeMap& seriesRanges)
//    {
//        auto view = spriteInfo.ForEach<&SpriteInfo::spriteName,
//                                       &SpriteInfo::seriesName,
//                                       &SpriteInfo::seriesIndex>(); 
//        if (std::ranges::empty(view))
//        {
//            return Void{};
//        }
//
//        size_t spriteIdx = 0;
//        size_t spanStart = 0;
//        bool inSpan = false;
//
//        for (const auto& [spriteName, seriesName, seriesIdx] : view)
//        {
//            if (spriteIndices.contains(spriteName))
//            {
//                // err
//            }
//
//            spriteIndices[spriteName] = spriteIdx;
//
//            if (!seriesName.empty())
//            {
//                if (!inSpan)
//                {
//                    // beginning a new series run
//                    if (seriesRanges.contains(seriesName))
//                    {
//                        // err
//                    }
//
//                    spanStart = spriteIdx;
//                    inSpan = true;
//                }
//            }
//            else
//            {
//                if (inSpan)
//                {
//                    // finishing a span
//                    seriesRanges[seriesName] = { spanStart, spriteIdx - 1 };
//                    inSpan = false;
//                }
//            }
//
//            if (inSpan && seriesIdx != spriteIndex - spanStart)
//            {
//                // err
//            }
//
//            ++spriteIdx;
//        }
//
//        // handle last span
//        if (inSpan)
//        {
//            size_t backIdx = spriteInfo.Size() - 1;
//
//            const auto& backSeriesName = spriteInfo.GetView<&SpriteInfo::seriesName>(
//                backIdx
//            );
//
//            seriesRanges[backSeriesName] = { spanStart, backIdx };
//        }
//    }
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



Result<SpriteAtlas> SpriteAtlas::Deserialize(SDL_Renderer* renderer, const nlohmann::json& j)
{
    SpriteAtlas atlas{ Handle<TextureAtlas>::Create() };

    try 
    {
        //j.get_to(atlas);
    }
    catch (nlohmann::json::parse_error& ex) 
    {
        return MAKE_ERROR(ex.what());
    }

    // validate texture size
    const size_t txSize = atlas.GetTextureSize();
    if (txSize == 0 || txSize > kMaxAtlasSize || !IsPowerOfTwo(txSize))
    {
        return MAKE_ERROR_FMT("Invalid atlas texture size: '{}'", txSize);
    }
    
    if (atlas.spriteInfo_.Empty())
    {
        return MAKE_ERROR("Deserialized sprite info was empty");
    }

    atlas.atlasTexture_ = MakeUniqueTexturePtr(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
        atlas.GetTextureSize(), atlas.GetTextureSize()
    );
    if (!atlas.atlasTexture_)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    SDL_SetTextureBlendMode(atlas.atlasTexture_.get(), SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(renderer, atlas.atlasTexture_.get());

    // setup render resources
    SDL_Surface* spriteSurface = nullptr;
    SDL_Texture* spriteTexture = nullptr;

    ScopedInvoker freeResources{ [&] {
        SDL_FreeSurface(spriteSurface);
        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
        SDL_SetRenderTarget(renderer, nullptr);
    } };

    for (const auto [plot, path] : atlas.spriteInfo_.ForEach<&SpriteInfo::plot, 
                                                             &SpriteInfo::filepath>())
    {
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
        if (spriteSurface->w != plot.rect.w || spriteSurface->h != plot.rect.h)
        {
            return MAKE_ERROR_FMT("Sprite surface dimensions: ({}, {}) did not match "
                "Sprite info plot dimensions: ({}, {})", spriteSurface->w,
                spriteSurface->h, plot.rect.w, plot.rect.h);
        }

        spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurface);
        if (!spriteTexture)
        {
            return MAKE_ERROR(SDL_GetError());
        }

        if (SDL_RenderCopy(renderer, spriteTexture, nullptr, &plot.rect) != 0)
        {
            return MAKE_ERROR(SDL_GetError());
        }
    }

    return atlas;  
}

SpriteDescriptorPackage SpriteAtlas::ExportSpriteDescriptors() const
{
    if (!IsLoaded())
    {
        return {};
    }

    std::vector<SpriteDescriptors> result;

    // reserve sizes
    result.reserve(seriesRanges_.size() + 1);

    result.emplace_back();
    auto getNonSeriesDescriptors = [&result] -> SpriteDescriptors& { 
        return result.front(); 
    };

    int numNonSeries = std::accumulate(seriesRanges_.begin(), seriesRanges_.end(), 0,
        [](int accum, const auto& pair) {
            return accum + (pair.second.max - pair.second.min);
        }
    );
    numNonSeries = spriteInfo_.Size() - numNonSeries;
    assert(numNonSeries >= 0);

    getNonSeriesDescriptors().data.reserve(static_cast<size_t>(numNonSeries));

    auto getCurrentSeriesDescriptors = [&result] -> SpriteDescriptors& {
        return (result.size() > 1) ? result.back() : result.emplace_back();
    };

    // fill result vector
    for (const auto [name, path, series] : spriteInfo_.ForEach<&SpriteInfo::spriteName,
                                                               &SpriteInfo::filepath,
                                                               &SpriteInfo::seriesName>())
    {
        if (series.empty())
        {
            getNonSeriesDescriptors().data.push_back({
                .spriteName = name,
                .filepath = path
            });
        }
        else 
        {
            if (series != getCurrentSeriesDescriptors().seriesName &&
                !getCurrentSeriesDescriptors().seriesName.empty())
            {
                result.push_back({ .seriesName = series });
            }

            getCurrentSeriesDescriptors().data.push_back({
                .spriteName = name,
                .filepath = path
            });
        }
    }

    return result;
}

Result<Sprite>
SpriteAtlas::LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
{
    SDL_SetRenderTarget(renderer, atlasTexture_.get());
    auto loadResult = LoadSpriteImpl(renderer, std::move(descriptor));
    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSprites(SDL_Renderer* renderer, SpriteDescriptors&& descriptors)
{
    SDL_SetRenderTarget(renderer, atlasTexture_.get());
    auto loadResult = LoadSpritesImpl(renderer, std::move(descriptors));
    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptors&& descriptors)
{
    if (descriptors.data.empty())
    {
        return MAKE_ERROR("Sprite descriptors were empty");
    }

    const bool isSpriteSeries = !descriptors.seriesName.empty();
    if (isSpriteSeries && seriesRanges_.contains(descriptors.seriesName))
    {
        //const auto& [min, max] = seriesRanges_.find(descriptors.seriesName)->second;

        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists",
            descriptors.seriesName);
    }

    spriteInfo_.Reserve(spriteInfo_.Size() + descriptors.data.size());

    std::vector<Sprite> sprites;
    sprites.reserve(descriptors.data.size());

    for (size_t i = 0; i < descriptors.data.size(); i++)
    {
        auto& newSprite = sprites.emplace_back();
        TRY_ASSIGN(newSprite, LoadSpriteImpl(renderer, 
                   std::move(descriptors.data[i])));

        if (isSpriteSeries)
        {
            auto [seriesName, seriesIdx] = 
                spriteInfo_.GetView<&SpriteInfo::seriesName,
                                    &SpriteInfo::seriesIndex>(newSprite.spriteIndex);

            seriesName = descriptors.seriesName;
            seriesIdx = i;
        }
    }
     
    if (isSpriteSeries)
    { 
        const size_t minSpriteIdx = sprites.front().spriteIndex;
        const size_t maxSpriteIdx = sprites.back().spriteIndex;

        assert(minSpriteIdx < maxSpriteIdx);
  
        std::string_view seriesNameRef = 
            spriteInfo_.GetView<&SpriteInfo::seriesName>(minSpriteIdx);

        seriesRanges_.emplace(RapidHash(seriesNameRef), Range<size_t>{
            .min = minSpriteIdx,
            .max = maxSpriteIdx
        });
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
    size_t spanStart = 0;
    bool inSpan = false;

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

        // assign to maps & handle series
        atlas.spriteIndices_.emplace(RapidHash(name), spriteIdx);

        if (!series.empty())
        {
            if (!inSpan)
            {
                // beginning a new series run
                if (atlas.seriesRanges_.contains(series))
                {
                    return MAKE_ERROR_FMT("Duplicate series name '{}'", series);
                }

                spanStart = spriteIdx;
                inSpan = true;
            }
        }
        else
        {
            if (inSpan)
            {
                // finishing a span
                atlas.seriesRanges_.emplace(RapidHash(series), Range<size_t>{ 
                    .min = spanStart, 
                    .max = spriteIdx - 1 
                });

                inSpan = false;
            }
        }

        if (inSpan && seriesIdx != spriteIdx - spanStart)
        {
            return MAKE_ERROR_FMT("invalid series index layout '{}'. "
                "expected index: '{}', actual index: '{}'", 
                spriteIdx - spanStart, seriesIdx);
        }

        ++spriteIdx;
    }         

    // close off last series
    if (inSpan)
    {
        size_t backIdx = spriteInfo.Size() - 1;

        const auto& backSeriesName = spriteInfo.GetView<&SpriteInfo::seriesName>(
            backIdx
        );
         
        atlas.seriesRanges_.emplace(RapidHash(backSeriesName), Range<size_t>{
            .min = spanStart, 
            .max = backIdx 
        });
    }

    return atlas;
}

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

Result<Void> SpriteAtlas::RebuildSourceTexture(SDL_Renderer* renderer)
{
    if (!IsLoaded())
    {
        LOG_WARNING("Sprite Atlas not loaded, did not rebuild texture");
        return Void{};
    }

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

    for (const auto& [plot, path] : spriteInfo_.ForEach<&SpriteInfo::plot, 
                                                        &SpriteInfo::filepath>())
    {
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
    }

    return Void{};
}

Sprite SpriteAtlas::GetSprite(std::string_view spriteName) const
{
    auto it = spriteIndices_.find(spriteName);
    if (it == spriteIndices_.end())
    {
        return {};
    }

    assert(it->second < spriteInfo_.Size());

    const auto& spriteNameRef = spriteInfo_.GetView<&SpriteInfo::spriteName>(it->second);
    // dont tolerate hash collisions
    assert(spriteName == spriteNameRef);

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
        const auto& seriesNameRef = spriteInfo_.GetView<&SpriteInfo::seriesName>(i);
        // dont tolerate hash collisions
        assert(seriesName == seriesNameRef);

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
    if (auto it = spriteIndices_.find(descriptor.spriteName); it != spriteIndices_.end())
    {
        // either return the loaded sprite or error if trying to reassign name to different png
        size_t spriteIdx = it->second;
        const auto& existingPath = spriteInfo_.GetView<&SpriteInfo::filepath>(spriteIdx);

        if (descriptor.filepath != existingPath)
        {
            return MAKE_ERROR_FMT("Sprite name '{}' already exists for a different filepath",
                descriptor.spriteName);
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
    spriteIndices_.emplace(RapidHash(nameRef), spriteIdx);

    return Sprite{
        .sourceAtlas = GetHandle(),
        .plot = plot,
        .spriteIndex = spriteIdx
    };
}
