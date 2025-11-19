#pragma once
#include "../FeatureFlags.h"

#if IMGUI_ENABLED

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <SDL.h>
#include "../core/Result.h"
#include "../core/commonObjects.h"

class GuiContext 
{
public:
	GuiContext(const GuiContext&) = delete;
	GuiContext& operator=(const GuiContext&) = delete;

	GuiContext(GuiContext&&) noexcept = default;
	GuiContext& operator=(GuiContext&&) noexcept = default;

	static Result<Void> Init(SDL_Window* window, SDL_Renderer* renderer);
	static void Exit();

	static void ProcessEvent(SDL_Event& ev);

	static void NewFrame();
	static void RenderPrepare();
	static void RenderPresent(SDL_Renderer* renderer);

	static bool IsInitialized() { return isInitialized_; }

private:
	GuiContext() = default;

	static inline bool isInitialized_ = false;
};

#endif