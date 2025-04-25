#pragma once
#include "B2Handle.h"
#include <SDL.h>
#include <numbers>
#include <algorithm>

static constexpr float kPixelsPerMeter = 30.0f;

inline constexpr float ToMeters(float pixels) { return pixels / kPixelsPerMeter; }
inline constexpr float ToPixels(float meters) { return meters * kPixelsPerMeter; }

inline constexpr SDL_FPoint ToSDLFPoint(const b2Vec2 vec)
{
    return { ToPixels(vec.x), ToPixels(vec.y) };
}
inline constexpr b2Vec2 ToB2Vec(const SDL_FPoint p)
{
    return { ToMeters(p.x), ToMeters(p.y) };
}

inline b2Rot AngleToB2Rot(float angleDeg)
{
    float angleRadians = angleDeg * (std::numbers::pi_v<float> / 180.0f);
    return b2MakeRot(angleRadians);
}

inline std::vector<b2Vec2> SDLFPointsToB2Vecs(const std::vector<SDL_FPoint>& points)
{
    std::vector<b2Vec2> vecPoints;
    vecPoints.reserve(points.size());

    std::transform(points.begin(), points.end(), std::back_inserter(vecPoints),
        [](const auto& p) { return ToB2Vec(p); });

    return vecPoints;
}
