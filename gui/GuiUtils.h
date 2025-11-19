#pragma once
#include "../FeatureFlags.h"

#if IMGUI_ENABLED

#include <imgui.h>
#include "../sdl/SDLUtils.h"

namespace util {

inline constexpr ImVec4 SDLColorToImVec4(SDL_Color c)
{
	return ImVec4{
		static_cast<float>(c.r) / 255.0f,
		static_cast<float>(c.g) / 255.0f,
		static_cast<float>(c.b) / 255.0f,
		static_cast<float>(c.a) / 255.0f
	};
}

template <SDLPointType P>
inline constexpr ImVec2 SDLPointToImVec2(P p)
{
	return ImVec2{
		static_cast<float>(p.x),
		static_cast<float>(p.y)
	};
}

} // util

#endif