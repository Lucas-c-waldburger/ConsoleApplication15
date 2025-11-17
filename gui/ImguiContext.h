#pragma once
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <SDL.h>
#include "../core/Result.h"
#include "../core/commonObjects.h"

class ImguiContext
{
public:
	ImguiContext(const ImguiContext&) = delete;
	ImguiContext& operator=(const ImguiContext&) = delete;

	ImguiContext(ImguiContext&&) noexcept = default;
	ImguiContext& operator=(ImguiContext&&) noexcept = default;

	static Result<Void> Init(SDL_Window* window, SDL_Renderer* renderer);
	static void Exit();

	static void ProcessEvent(SDL_Event& ev);

	static void NewFrame();
	static void RenderPrepare();
	static void RenderPresent(SDL_Renderer* renderer);

	static bool IsInitialized() { return isInitialized_; }

private:
	ImguiContext() = default;

	static inline bool isInitialized_ = false;
};

//class ImguiWindow
//{
//public:
//	//struct Args
//	//{
//	//	const char* title;
//	//	bool* pOpen = nullptr;
//	//	ImGuiWindowFlags flags = 0;
//	//};
//
//	ImguiWindow(const char* title, bool* pOpen, ImGuiWindowFlags flags);
//	//~ImguiWindow();
//
//	template <typename Fn> requires std::invocable<Fn>
//	void operator()(Fn&& fn)
//	{
//		std::invoke(fn);
//		ImGui::End();
//	}
//
//
//	//[[no_discard]] static Scope Create(const char* title, bool* pOpen = nullptr, 
//	//								   ImGuiWindowFlags flags = 0);
//
//private:
//};