#pragma once
#include "../events/EventBus2.h"
#include "../inputs/controller/GameController.h"
#include "../components/builder/ColliderComponentBuilder.h"
#include "../ecs/Ecs.h"
#include "../ecs/EntityPhysics.h"

namespace test {

class ColliderMaker
{
public:
    static ColliderMaker Create(B2World& world, EventBus& bus)
    {
        ColliderMaker colliderMaker{};
        colliderMaker.Init(world, bus);

        return colliderMaker;
    }

    void Draw()
    {
         

    }

    Result<B2Shape> AddShape(Entity& bodyEnt, const ColliderSettings& settings,
                             EntityPhysics::Local local = {})
    {
        assert(world_);

        if (points_.size() < 3)
        {
            LOG_ERROR("Must have at least 3 points to create polygon");

            return B2Shape{};
        }

        auto phys = bodyEnt.GetPhysics(*world_);
        if (!phys.HasBody())
        {
            return MAKE_ERROR("Source entity has no body");
        }
        
        return phys.AddColliderPoly(points_, settings, local); 
    }

    void SetShapeType(B2Shape::Type type) 
    { 
        if (shapeType_ != type)
        {
            points_.clear();
            shapeType_ = type;
        }
    }

private:
    void Init(B2World& world, EventBus& bus)
    {
        using enum MouseInputSource;

        world_ = &world;

        addPointToken_ = bus.ConnectToInput(LeftButton, 
            [this](const events::MouseInput& ev) {
                if (!inDrawMode_)
                {
                    return;
                }
                if (ev.input.state != InputState::Pressed)
                {
                    return;
                }
                auto [x, y] = ev.values.cursor.absolutePos;
                points_.emplace_back(static_cast<float>(x), static_cast<float>(y));
            });

        removePointToken_ = bus.ConnectToInput(RightButton,
            [this](const events::MouseInput& ev) {
                if (ev.input.state != InputState::Pressed)
                {
                    return;
                }
                if (!points_.empty())
                {
                    points_.pop_back();
                }
            });

        updateMousePosToken_ = bus.ConnectToInput(Cursor,
            [this](const events::MouseInput& ev) {
                auto [x, y] = ev.values.cursor.absolutePos;
                currentMousePos_ = { static_cast<float>(x), static_cast<float>(y) };
            });

        updateMousePosToken_ = bus.ConnectToInput(MiddleButton,
            [this](const events::MouseInput& ev) {
                if (ev.input.state != InputState::Pressed)
                {
                    return;
                }
                inDrawMode_ = !inDrawMode_;
            });
    }

    B2World* world_ = nullptr;
    SDL_FPoint currentMousePos_ = { 0.0f, 0.0f };
    SignalToken addPointToken_;
    SignalToken removePointToken_;
    SignalToken updateMousePosToken_;
    std::vector<SDL_FPoint> points_;
    bool inDrawMode_ = false;
    B2Shape::Type shapeType_ = B2Shape::Type::Polygon;
};

template <typename T> requires std::is_arithmetic_v<T>
T MapControllerJoystickToScalar(SDL_FPoint axisValue, Range<T> scalarRange, bool invert = false)
{
    const float absX = std::abs(axisValue.x);
    const float absY = std::abs(axisValue.y);

    const float maxVal = std::max(absX, absY);
    const float minVal = std::min(absX, absY);

    // Approximate Euclidean magnitude
    const float approxMag = maxVal + 0.5f * minVal;

    constexpr float maxApprox = static_cast<float>(GameController::kAxisMax) + 0.5f *
                                static_cast<float>(GameController::kAxisMax);

    float scaled = (approxMag / maxApprox) * 
                   static_cast<float>(scalarRange.max - scalarRange.min);

    if (invert) 
    {
        scaled = static_cast<float>(scalarRange.max - scalarRange.min) - scaled;
    }

    scaled += static_cast<float>(scalarRange.min); // shift to range.min

    return static_cast<T>(std::clamp(
        scaled,
        static_cast<float>(scalarRange.min),
        static_cast<float>(scalarRange.max)));
}











} // test