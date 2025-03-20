#pragma once
#include "Core.h"
#include "SDLite.h"
#include "GlyphAtlas.h"
#include "SpriteSeriesAtlas.h"
#include <unordered_set>
#include <optional>
#include <variant>

// eventually move enums/nested structs into some component helper thing

struct Kinematics : BaseComponent<Kinematics>
{
    SDL_FPoint velocity = { 0.0f, 0.0f };
    SDL_FPoint acceleration = { 0.0f, 0.0f };
    float drag = 0.0f;
};

struct Spatial : BaseComponent<Spatial>
{
    SDL_FPoint position = { 0.0f, 0.0f };
    Dimensions<float> dimensions = { 0.0f, 0.0f };
};

struct Transform : BaseComponent<Transform>
{
    float scale = 1.0f;
    float rotation = 0.0f;
    SDL_FPoint offset = { 0.0f, 0.0f };
};

struct Parent : BaseComponent<Parent>
{
    Entity_t parentEntity = kInvalidEntity;
};

struct Children : BaseComponent<Children>
{
    std::unordered_set<Entity_t> childEntities;
};

struct Tags : BaseComponent<Tags>
{
    std::unordered_set<std::string> tags;
};

//// RENDERABLES ////

/* TODO : IMPLEMENT */
//struct RenderData

struct Renderable : BaseComponent<Renderable>
{
    struct Text
    {
        enum Alignment
        {
            Left,
            Center,
            Right
        };

        Handle<GlyphAtlas> sourceAtlas;
        std::string text;
        Alignment align;
    };

    struct Sprite
    {
        Handle<SpriteSeriesAtlas> sourceAtlas;
        std::string seriesName;
        int currentIndex = -1;
    };

    std::variant<Text, Sprite> renderData;
    int drawOrder = -1;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
};

//////////////////////

struct RenderableText : Renderable
{
    std::string text;
};

struct RenderText : BaseComponent<RenderText>, 
                    Renderable
{
    Handle<GlyphAtlas> sourceAtlas;
    std::string textString;

};

struct RenderSprite : BaseComponent<RenderSprite>, 
                      Renderable
{
    Handle<SpriteSeriesAtlas> sourceAtlas;
    std::string seriesName;     
    int currentIndex = -1;
};

template <typename T>
concept RenderableComponentType = (ComponentType<T> && std::derived_from<T, Renderable>);


template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);

template <ComponentType...Ts> using ComponentRegistryTemplate = TypeList<Ts...>;

using ComponentRegistry = ComponentRegistryTemplate<
    Kinematics,
    Spatial,
    Transform,
    Parent,
    Children,
    Tags,
    Renderable
>;


static SDL_Rect MakeTransformedRect(const Spatial& spatial, const Transform& tf)
{
    float scaledW = spatial.dimensions.w * tf.scale;
    float scaledH = spatial.dimensions.h * tf.scale;

    return SDL_Rect{
        static_cast<int>(spatial.position.x - (scaledW / 2.0f)),
        static_cast<int>(spatial.position.y - (scaledH / 2.0f)),
        static_cast<int>(scaledW),
        static_cast<int>(scaledH)
    };
}




//static SDL_Rect SpatialToRect(const Spatial& spatial)
//{
//    return SDL_Rect{
//        static_cast<int>(spatial.position.x - (spatial.dimensions.w / 2.0f)),
//        static_cast<int>(spatial.position.y - (spatial.dimensions.h / 2.0f)),
//        static_cast<int>(spatial.dimensions.w),
//        static_cast<int>(spatial.dimensions.h)
//    };
//}
//
//static void ApplyTransformToRect(SDL_Rect& rect, const Transform& tf)
//{
//    int scaledW = static_cast<int>(static_cast<float>(rect.w) * tf.scale);
//    int scaledH = static_cast<int>(static_cast<float>(rect.h) * tf.scale);
//
//    int diffW = rect.w - scaledW;
//    int diffH = rect.h = scaledH;
//
//
//}