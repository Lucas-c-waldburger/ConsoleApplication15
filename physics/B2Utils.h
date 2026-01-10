#pragma once
#include "B2Handle.h"
#include <SDL.h>
#include <numbers>
#include <algorithm>

//inline constexpr float kPI = 3.14159265358979323846f;

inline constexpr float kPixelsPerMeter = 30.0f;

inline constexpr float ToMeters(float pixels) { return pixels / kPixelsPerMeter; }
inline constexpr float ToPixels(float meters) { return meters * kPixelsPerMeter; }

inline constexpr SDL_FPoint ToSDLFPoint(const b2Vec2 vec)
{
    return { vec.x, vec.y };
}
inline constexpr b2Vec2 ToB2Vec(const SDL_FPoint p)
{
    return { p.x, p.y };
}

inline constexpr SDL_FPoint ToSDLFPointScaled(const b2Vec2 vec)
{
    return { ToPixels(vec.x), ToPixels(vec.y) };
}
inline constexpr b2Vec2 ToB2VecScaled(const SDL_FPoint p)
{
    return { ToMeters(p.x), ToMeters(p.y) };
}

inline b2Rot AngleToB2Rot(float angleDeg)
{
    float angleRadians = angleDeg * (std::numbers::pi_v<float> / 180.0f);
    return b2MakeRot(angleRadians);
}

inline constexpr float RadiansToAngleDegrees(float rad)
{
    return rad * (180.0f / std::numbers::pi_v<float>);
}

inline std::vector<b2Vec2> SDLFPointsToB2Vecs(const std::vector<SDL_FPoint>& points)
{
    std::vector<b2Vec2> vecPoints;
    vecPoints.reserve(points.size());

    std::transform(points.begin(), points.end(), std::back_inserter(vecPoints),
        [](const auto& p) { return ToB2VecScaled(p); });

    return vecPoints;
}

inline SDL_FRect B2AABBToSDLFRect(const b2AABB& aabb)
{
    SDL_FPoint center = ToSDLFPointScaled(b2AABB_Center(aabb));
    SDL_FPoint extents = ToSDLFPointScaled(b2AABB_Extents(aabb));

    return {
        .x = center.x - extents.x,
        .y = center.y - extents.y,
        .w = extents.x * 2.0f,
        .h = extents.y * 2.0f
    };
}

