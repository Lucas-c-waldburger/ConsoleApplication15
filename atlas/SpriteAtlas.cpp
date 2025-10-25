#include "SpriteAtlas.h"
#include "../core/commonObjects.h"
#include <SDL_image.h>
#include <cassert>
#include <ranges>
#include "PackingTools.h"
#include "../core/Algorithms.h"
#include "../core/ScopedInvoker.h"

namespace {

constexpr bool IsPowerOfTwo(size_t x) noexcept 
{
    return x != 0 && (x & (x - 1)) == 0;
}

} // unnamed namespace

const SpriteInfo SpriteAtlas::kInvalidSpriteInfo{};

Result<Sprite> 
SpriteAtlas::LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
{
    SDL_SetRenderTarget(renderer, atlasTexture_.get());
    auto loadResult = LoadSpriteImpl(renderer, std::move(descriptor));
    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSprites(SDL_Renderer* renderer, SpriteDescriptorPackage&& package)
{
    SDL_SetRenderTarget(renderer, atlasTexture_.get());
    auto loadResult = LoadSpritesImpl(renderer, std::move(package));
    SDL_SetRenderTarget(renderer, nullptr);

    return loadResult;
}

Result<std::vector<Sprite>> 
SpriteAtlas::LoadSpritesImpl(SDL_Renderer* renderer, SpriteDescriptorPackage&& package)
{
    if (package.descriptors.empty())
    {
        return MAKE_ERROR("Sprite descriptor package was empty");
    }

    const bool isSpriteSeries = !package.seriesName.empty();
    if (isSpriteSeries && spriteSeriesRanges_.contains(package.seriesName))
    {
        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists",
            package.seriesName);
    }

    spriteInfo_.reserve(spriteInfo_.size() +
                        package.descriptors.size());

    std::vector<Sprite> sprites;
    sprites.reserve(package.descriptors.size());

    SDL_SetRenderTarget(renderer, atlasTexture_.get());

    for (size_t i = 0; i < package.descriptors.size(); i++)
    {
        auto& newSprite = sprites.emplace_back();
        TRY_ASSIGN(newSprite, LoadSpriteImpl(renderer, 
            std::move(package.descriptors[i])));

        if (isSpriteSeries)
        {
            auto& info = spriteInfo_[newSprite.spriteIndex];
            info.seriesName = package.seriesName;
            info.seriesIndex = i;
        }
    }

    if (isSpriteSeries)
    { 
        assert(sprites.front().spriteIndex < sprites.back().spriteIndex);

        spriteSeriesRanges_[std::move(package.seriesName)] = {
            .min = static_cast<size_t>(sprites.front().spriteIndex),
            .max = static_cast<size_t>(sprites.back().spriteIndex)
        };
    }

    return sprites;
}

Result<SpriteAtlas> SpriteAtlas::Create(SDL_Renderer* renderer, size_t size)
{
    SpriteAtlas atlas{ Handle<NewTextureAtlas>::Create() };

    size = IsPowerOfTwo(size) ? size : 
           size > kMaxAtlasSize ? kMaxAtlasSize :
           GetNextPowerOfTwo(static_cast<int>(size));

    atlas.atlasTexture_ = MakeUniqueTexturePtr(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size
    );
    if (!atlas.atlasTexture_)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    atlas.binPack_.Init(size, size, false);

    return atlas;
}

const SpriteInfo& SpriteAtlas::GetSpriteInfo(const Sprite& sprite) const
{
    auto validated = ValidateSprite(sprite);
    if (!validated.Success())
    {
        LOG_ERROR(validated.GetError());
        return kInvalidSpriteInfo;
    }

    return spriteInfo_[sprite.spriteIndex];
}

Result<Void> SpriteAtlas::ValidateSprite(const Sprite& sprite) const
{
    if (sprite.sourceAtlas != GetAtlasHandle())
    {
        return MAKE_ERROR("Sprite does not belong to this atlas");
    }
    if (sprite.spriteIndex >= sprites_.size())
    {
        return MAKE_ERROR_FMT("Sprite index '{}' out of range", sprite.spriteIndex);
    }
    if (sprite.plot.rect.w > 0 && sprite.plot.rect.h > 0)
    {
        return MAKE_ERROR_FMT("Sprite atlas plot dimensions invalid: ({}, {})",
            sprite.plot.rect.w, sprite.plot.rect.h);
    }

    return Void{};
}

Sprite SpriteAtlas::GetSprite(std::string_view spriteName) const
{
    for (size_t i = 0; i < spriteInfo_.size(); i++)
    {
        if (spriteInfo_[i].spriteName == spriteName)
        {
            assert(i < sprites_.size());
            return sprites_[i];
        }
    }

    return {};
}

std::vector<Sprite> SpriteAtlas::GetSpriteSeries(std::string_view seriesName) const
{
    auto it = spriteSeriesRanges_.find(seriesName);
    if (it == spriteSeriesRanges_.end())
    {
        return {};
    }

    const auto [start, end] = it->second;

    assert(end > start);
    assert(end < sprites_.size());

    std::vector<Sprite> sprites;
    sprites.reserve(end - start);

    for (size_t i = start; i <= end; i++)
    {
        sprites.emplace_back(sprites_[i]);
    }

    return sprites;
}

Result<Sprite> SpriteAtlas::LoadSpriteImpl(SDL_Renderer* renderer,
                                           SpriteDescriptor&& descriptor)
{
    if (!atlasTexture_)
    {
        return MAKE_ERROR("Atlas texture was null");
    }
    assert(GetAtlasHandle().IsValid());

    SDL_Surface* spriteSurface = nullptr;
    SDL_Texture* spriteTexture = nullptr;

    ScopedInvoker freeResources{ [&] { 
        SDL_FreeSurface(spriteSurface);
        if (spriteTexture) { SDL_DestroyTexture(spriteTexture); }
        SDL_SetRenderTarget(renderer, nullptr);
    }};
    
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

    assert(spriteInfo_.size() == sprites_.size());

    spriteInfo_.emplace_back(SpriteInfo{
        .spriteName = std::move(descriptor.spriteName),
        .filepath = std::move(descriptor.filepath)
    });

    auto& sprite = sprites_.emplace_back(Sprite{
        .sourceAtlas = GetAtlasHandle(),
        .plot = plot,
        .spriteIndex = sprites_.size()
    });

    return sprite;
}
