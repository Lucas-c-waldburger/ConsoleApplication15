#pragma once
#include "Core.h"
#include "SDLite.h"
#include "GlyphAtlas.h"
#include "SpriteSeriesAtlas.h"
#include "inputs/GameController.h"
#include <array>
#include <unordered_set>
#include <optional>
#include <variant>

// eventually move enums/nested structs into some component helper thing

// SPATIAL
struct Spatial : BaseComponent<Spatial, 1>
{
    SDL_FPoint position = {0.0f, 0.0f};
    Dimensions<float> dimensions = { 0.0f, 0.0f };
};

// TRANSFORM
struct Transform : BaseComponent<Transform, 2>
{
    SDL_FPoint scale = { 1.0f, 1.0f };
    float rotation = 0.0f;
    SDL_FPoint offset = { 0.0f, 0.0f };
};

// PHYSICS
struct Force
{
    SDL_FPoint vector = { 0, 0 };
    float duration = 0.0;
};

struct ForceAccumulator
{
    std::vector<Force> forces;
    float maxForce = 0.0;
};

struct Physics : BaseComponent<Physics, 3>
{
    SDL_FPoint velocity = { 0.0f, 0.0f };
    SDL_FPoint acceleration = { 0.0f, 0.0f };
    float mass = 0.0f;
    float drag = 0.0f;
    ForceAccumulator forceAccumulator = {};
};

// RELATION COMPONENTS
struct Parent : BaseComponent<Parent, 4>
{
    Entity_t parentEntity = kInvalidEntity;
};

struct Children : BaseComponent<Children, 5>
{
    std::unordered_set<Entity_t> childEntities;
};

// TAGS
struct Tags : BaseComponent<Tags, 6>
{
    std::unordered_set<std::string> tags;
};

//// RENDERABLES ////

/* TODO */
// separate atlases into some store interface seperate from renderSystem. 
// a renderable "Text" or "Sprite" should hold the atlas rect itself
// its fine if you have to go back in with the handle to do the final draw 

struct Renderable : BaseComponent<Renderable, 7>
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
        bool scaleToFit = false;
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

// COLLIDER
struct Collider : public BaseComponent<Collider, 8>
{
    SDL_FPoint position = { 0.0f, 0.0f };
    Dimensions<float> dimensions = { 0.0f, 0.0f };
};

// GAME CONTROLLER STATE
struct AxisInputState
{
    SDL_FPoint value = { 0.0f, 0.0f };
    uint32_t timestamp = 0;
    uint8_t state = 0;
    uint32_t stateDuration = 0;
};

struct ButtonInputState
{
    SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
    uint32_t timestamp = 0;
    uint8_t state = 0;
    uint32_t stateDuration = 0;
};

struct GameControllerState : public BaseComponent<GameControllerState, 9>
{
    enum State : uint8_t { None, Pressed, Released, Held };

    SDL_JoystickID joystickID = GameController::kInvalidJoystickID;
    HandedPair<AxisInputState> axisInput = {};
    std::array<ButtonInputState, SDL_CONTROLLER_BUTTON_MAX> buttonInput = {};
};



//////////////////////
template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);


static SDL_Rect MakeTransformedRect(const Spatial& spatial, const Transform& tf)
{
    float scaledW = spatial.dimensions.w * tf.scale.x;
    float scaledH = spatial.dimensions.h * tf.scale.y;

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