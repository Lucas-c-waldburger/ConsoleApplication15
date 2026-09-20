#include "GuiTexture.h"

#if IMGUI_ENABLED
#include "../Fixtures.h"

namespace ui {

GuiTexture GuiTextureConverter::FromRenderTarget(const RenderTarget& renderTarget)
{
	SDL_Texture* tx = renderTarget;
	assert(tx);

	return GuiTexture{
		.textureId = reinterpret_cast<ImTextureID>(tx),
		.size = ImVec2(
			static_cast<float>(renderTarget.width),
			static_cast<float>(renderTarget.height)),
		.uv0 = ImVec2(0.0f, 0.0f),
		.uv1 = ImVec2(1.0f, 1.0f)
	};
}

} // ui

#endif