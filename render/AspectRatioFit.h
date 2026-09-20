#pragma once
#include <cstdint>
#include <utility>

enum class AspectRatioFit : uint8_t
{
	Letterbox,
	Fill,
	Stretch
};

inline constexpr float GetAspectRatioFitScale(AspectRatioFit fit,
											  float targetW, float targetH,
											  float srcW, float srcH)
{
    assert(srcW > 0.0f && srcH > 0.0f);

    const float sx = targetW / srcW;
    const float sy = targetH / srcH;

    switch (fit)
    {
    case AspectRatioFit::Stretch:
        return 1.0f;

    case AspectRatioFit::Letterbox:
        return std::min(sx, sy);

    case AspectRatioFit::Fill: default:
        return std::max(sx, sy);
    }

    std::unreachable();
}