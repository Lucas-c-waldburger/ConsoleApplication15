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
    ImVec2 size;
    ImVec2 uv0;
    ImVec2 uv1;

    static constexpr GuiTexture Default()
    {
        return GuiTexture{
            .textureId = 0,
            .size = ImVec2(0, 0),
            .uv0 = ImVec2(0, 0),
            .uv1 = ImVec2(1, 1)
        };
    }
};

class GuiTextureConverter
{
public:
    explicit GuiTextureConverter(const TextureRepository& repo) : repo_(repo) {}

    GuiTexture FromSprite(const Sprite& sprite) const
    {
        if (!sprite.resourceHandle.IsValid())
        {
            return GuiTexture::Default();
        }

        return MakeGuiTexture(repo_.GetSourceTexture(sprite.resourceHandle), sprite.plot);
    }

    GuiTexture FromSprite(std::string_view spriteName) const
    {
        return FromSprite(repo_.GetSpriteAtlas().GetSprite(spriteName));
    }

    GuiTexture FromGlyph(const Glyph& glyph, std::string_view fontName) const
    {
        const auto& fnt = repo_.GetFontAtlas().GetFont(fontName);
        if (!fnt.IsLoaded())
        {
            return GuiTexture::Default();
        }

        return MakeGuiTexture(fnt.GetSourceTexture(), glyph.plot);
    }

    GuiTexture FromGlyph(char c, std::string_view fontName) const
    {
        const auto& fnt = repo_.GetFontAtlas().GetFont(fontName);
        if (!fnt.IsLoaded())
        {
            return GuiTexture::Default();
        }

        const auto glyph = fnt.GetGlyph(c);
        
        return MakeGuiTexture(fnt.GetSourceTexture(), glyph.plot);
    }

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
            return GuiTexture::Default();
        }

        const auto [txW, txH] = GetTextureSize(srcTexture);
        assert(txW > 0 && txH > 0);

        float x = static_cast<float>(plot.rect.x);
        float y = static_cast<float>(plot.rect.y);
        float w = static_cast<float>(plot.rect.w);
        float h = static_cast<float>(plot.rect.h);

        if (!(w > 0.0f && h > 0.0f))
        {
            return {};
        }

        return GuiTexture{
            .textureId = (ImTextureID)srcTexture,
            .size = ImVec2(w, h),
            .uv0 = ImVec2(x / txW, y / txH),
            .uv1 = ImVec2((x + w) / txW, (y + h) / txH)
        };
    }

    const TextureRepository& repo_;
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
    if (tx.textureId == 0)
    {
        return;
    }

    ImGui::Image(tx.textureId, tx.size, tx.uv0, tx.uv1);
}

inline bool GuiImageButton(std::string_view id, const GuiTexture& tx, 
    const ImVec4& bgClr = ImVec4(0, 0, 0, 0),
    const ImVec4& tintClr = ImVec4(1, 1, 1, 1))
{
    if (tx.textureId == 0)
    {
        return false;
    }

    return ImGui::ImageButton(id.data(), tx.textureId, tx.size, tx.uv0, tx.uv1,
                              bgClr, tintClr);
}

//inline bool GuiImageButton(std::string_view id, const GuiTexture& tx, const SDL_Color& bgClr = {0, 0, 0, 0}, 
//                           const SDL_Color& tintClr = { 255, 255, 255, 255 })
//{
//    if (tx.textureId == 0)
//    {
//        return false;
//    }
//
//    return ImGui::ImageButton(id.data(), tx.textureId, tx.size, tx.uv0, tx.uv1,
//                              SDLToGuiColor(bgClr), SDLToGuiColor(tintClr));
//}

} // ui


#endif