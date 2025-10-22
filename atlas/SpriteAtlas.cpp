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

Result<std::vector<Texture>> 
SpriteAtlas::LoadSpriteSeries(SDL_Renderer* renderer, std::string_view seriesName,
                              std::vector<SpriteDescriptor>&& descriptors)
{
    if (descriptors.size() <= 1)
    {
        return MAKE_ERROR_FMT("Invalid sprite descriptors vector size: '{}'",
            descriptors.size());
    }

    if (spriteSeriesReferences_.contains(seriesName))
    {
        return MAKE_ERROR_FMT("Sprite series with name '{}' already exists", seriesName);
    }

    spriteDescriptors_.reserve(spriteDescriptors_.size() + descriptors.size());

    std::vector<Texture> textures;
    textures.reserve(descriptors.size());

    for (auto&& descriptor : descriptors)
    {
        auto& newTexture = textures.emplace_back();
        TRY_ASSIGN(newTexture, LoadImpl(renderer, std::move(descriptor)));
    }

    spriteSeriesReferences_[seriesName] = {
        .min = static_cast<size_t>(textures.front().plotIndex),
        .max = static_cast<size_t>(textures.front().plotIndex)
    };

    return textures;
}

Result<Texture> SpriteAtlas::LoadSprite(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
{
    return LoadImpl(renderer, std::move(descriptor));
}

Result<Texture> SpriteAtlas::LoadSprites(SDL_Renderer* renderer, std::vector<SpriteDescriptor>&& descriptors)
{
    if (descriptors.size() <= 1)
    {
        return MAKE_ERROR_FMT("Invalid sprite descriptors vector size: '{}'",
            descriptors.size());
    }

    spriteDescriptors_.reserve(spriteDescriptors_.size() + descriptors.size());

    std::vector<Texture> textures;
    textures.reserve(descriptors.size());

    for (auto&& descriptor : descriptors)
    {
        auto& newTexture = textures.emplace_back();
        TRY_ASSIGN(newTexture, LoadImpl(renderer, std::move(descriptor)));
    }

    spriteSeriesReferences_[seriesName] = {
        .min = static_cast<size_t>(textures.front().handle.GetPlotIndex()),
        .max = static_cast<size_t>(textures.front().handle.GetPlotIndex())
    };
}

Result<SpriteAtlas> SpriteAtlas::Create(SDL_Renderer* renderer, size_t size)
{
    SpriteAtlas atlas{};

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

    atlas.SetHandle(Handle<NewTextureAtlas>::Create(TextureType::Sprite));

    return atlas;
}

const SpriteDescriptor* SpriteAtlas::GetSpriteDescriptor(const TextureHandle& handle) const
{
    if (!handle.IsValid() || handle.GetPlotIndex() >= plots_.size())
    {
        return nullptr;
    }

    return &spriteDescriptors_[handle.GetPlotIndex()];
}

TextureHandle SpriteAtlas::GetSpriteHandle(std::string_view spriteName) const
{
    for (size_t i = 0; i < spriteDescriptors_.size(); i++)
    {
        if (spriteDescriptors_[i].spriteName == spriteName)
        {
            return TextureHandle::Create(GetAtlasHandle(), i);
        }
    }

    return {};
}

AtlasPlot SpriteAtlas::GetAtlasPlot(const TextureHandle& handle) const
{
    if (!handle.IsValid() || handle.GetPlotIndex() >= plots_.size())
    {
        return {};
    }

    return plots_[handle.GetPlotIndex()];
}

std::vector<TextureHandle> SpriteAtlas::GetSpriteSeriesHandles(std::string_view seriesName) const
{
    auto it = spriteSeriesReferences_.find(seriesName);
    if (it == spriteSeriesReferences_.end())
    {
        return {};
    }

    const auto [start, end] = it->second;

    assert(end > start);
    assert(end < plots_.size());

    std::vector<TextureHandle> sprites;
    sprites.reserve(end - start);

    for (size_t i = start; i <= end; i++)
    {
        sprites.emplace_back(TextureHandle::Create(GetAtlasHandle(), i));
    }

    return sprites;
}

Result<Texture> SpriteAtlas::LoadImpl(SDL_Renderer* renderer, SpriteDescriptor&& descriptor)
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

    SDL_SetRenderTarget(renderer, atlasTexture_.get()); 
    if (SDL_RenderCopy(renderer, spriteTexture, nullptr, &plot.rect) != 0)
    {
        return MAKE_ERROR(SDL_GetError());
    }

    assert(spriteDescriptors_.size() == plots_.size());

    Texture texture{
        .handle = TextureHandle::Create(GetAtlasHandle(), plots_.size()),
        .plot = plot
    };

    spriteDescriptors_.emplace_back(std::move(descriptor));
    plots_.emplace_back(plot);

    return texture;
}
