#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <string_view>
#include <filesystem>
#include "../../deps/function2/function2.hpp"
#include "../../ecs/Ecs.h"
#include "../../atlas/NewTextureRepository.h"

namespace ui {

struct GuiTexture
{
    ImTextureID textureId = 0;
    ImVec2 size{ 0, 0 };
    ImVec2 uv0{ 0, 0 };
    ImVec2 uv1{ 1, 1 };

    constexpr bool IsValid() const noexcept { return textureId != 0; }
};

class GuiTextureConverter
{
public:
    GuiTextureConverter() = default;
    explicit GuiTextureConverter(const TextureRepository& repo) : repo_(&repo) {}

    GuiTexture FromSprite(const Sprite& sprite) const
    {
        if (!repo_)
        {
            return {};
        }

        if (!sprite.resourceHandle.IsValid())
        {
            return {};
        }

        return MakeGuiTexture(repo_->GetSourceTexture(sprite.resourceHandle), sprite.plot);
    }

    GuiTexture FromSprite(std::string_view spriteName) const
    {
        if (!repo_)
        {
            return {};
        }

        return FromSprite(repo_->GetSpriteAtlas().GetSprite(spriteName));
    }

    GuiTexture FromGlyph(const Glyph& glyph, std::string_view fontName) const
    {
        if (!repo_)
        {
            return {};
        }

        const auto& fnt = repo_->GetFontAtlas().GetFont(fontName);
        if (!fnt.IsLoaded())
        {
            return {};
        }

        return MakeGuiTexture(fnt.GetSourceTexture(), glyph.plot);
    }

    GuiTexture FromGlyph(char c, std::string_view fontName) const
    {
        if (!repo_)
        {
            return {};
        }

        const auto& fnt = repo_->GetFontAtlas().GetFont(fontName);
        if (!fnt.IsLoaded())
        {
            return {};
        }

        const auto glyph = fnt.GetGlyph(c);
        
        return MakeGuiTexture(fnt.GetSourceTexture(), glyph.plot);
    }

    void SetTextureRepository(const TextureRepository& repo) noexcept { repo_ = &repo; }

private:
    static Dimensions<float> GetTextureSize(SDL_Texture* tx) 
    {
        SDL_Point size{ 0, 0 };
        SDL_QueryTexture(tx, NULL, NULL, &size.x, &size.y);

        return { static_cast<float>(size.x), static_cast<float>(size.y) };
    }

    GuiTexture MakeGuiTexture(SDL_Texture* srcTexture, const AtlasPlot& plot) const
    {
        if (!srcTexture)
        {
            return {};
        }

        const auto [txW, txH] = GetTextureSize(srcTexture);
        assert(txW > 0 && txH > 0);

        const auto& [x, y, w, h] = plot.rect;
        if (!(w > 0 && h > 0))
        {
            return {};
        }

        return GuiTexture{
            0,
            ImVec2{x, y},
            ImVec2{x / txW, y / txH},
            ImVec2{(x + w) / txW, (y + h) / txH}
        };
    }

    const TextureRepository* repo_ = nullptr;
};

constexpr inline ImVec4 SDLToGuiColor(const SDL_Color& clr)
{
    static constexpr auto cast = [](uint8_t v) { return static_cast<float>(v) / 255.0f; };

    return { cast(clr.r), cast(clr.g), cast(clr.b), cast(clr.a) };
}

constexpr inline ImVec4 RGBToGuiColor(const RGB& clr, uint8_t alpha = 255)
{
    static constexpr auto cast = [](uint8_t v) { return static_cast<float>(v) / 255.0f; };

    return { cast(clr.r), cast(clr.g), cast(clr.b), cast(alpha) };
}

template <SDLPointType P>
constexpr inline ImVec2 SDLToGuiVec2(P p)
{
    return { static_cast<float>(p.x), static_cast<float>(p.y) };
}

template <typename T>
constexpr inline ImVec2 DimensionsToGuiVec2(Dimensions<T> dims)
{
    return { static_cast<float>(dims.w), static_cast<float>(dims.h) };
}

template <typename T>
inline std::string MakeGuiStringID(T* ptr)
{
    return std::format("##{:p}", static_cast<const void*>(ptr));
}

inline void GuiImage(const GuiTexture& tx)
{
    if (!tx.IsValid())
    {
        return;
    }

    ImGui::Image(tx.textureId, tx.size, tx.uv0, tx.uv1);
}

inline void GuiImageButton(const GuiTexture& tx, const SDL_Color& bgClr = {0, 0, 0, 0}, 
                           const SDL_Color& tintClr = { 255, 255, 255, 255 })
{
    if (!tx.IsValid())
    {
        return;
    }

    std::string id = MakeGuiStringID(&tx);

    ImGui::ImageButton(id.c_str(), tx.textureId, tx.size, tx.uv0, tx.uv1,
                       SDLToGuiColor(bgClr), SDLToGuiColor(tintClr));
}

} // ui


#endif