#pragma once
#include "../events/EventBus2.h"
#include "../inputs/controller/GameController.h"
#include "../ecs/Ecs.h"

namespace test {

//class ControllerInputDriver
//{
//public:
//    using Source = GameControllerInputSource;
//
//    ControllerInputDriver(EventBus2& bus, Entity entity) : bus_(bus), entity_(entity) {}
//
//    template <typename Fn>
//    void OnButtonPress(Source src, Fn&& fn)
//    {
//        entity_.AddComponent<SignalTokenStorage>().signalTokens.emplace_back(
//            bus_.ConnectToInput(src, std::forward<Fn>(fn));
//        );
//    }
//
//
//
//private:
//    EventBus2& bus_;
//    Entity entity_;
//};

//const float kThumbstickAxisMaxMagnitude = 
//	std::sqrt(GameController::kAxisMax * GameController::kAxisMax + 
//			  GameController::kAxisMax * GameController::kAxisMax);

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