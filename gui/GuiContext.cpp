#include "GuiContext.h"

#if IMGUI_ENABLED

#include "GuiUtils.h"

void GuiContext::Exit()
{
    if (!isInitialized_) { return; }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

Result<Void> GuiContext::Init(SDL_Window* window, SDL_Renderer* renderer)
{
    if (isInitialized_) { return Void{}; }

    if (!window)   { return MAKE_ERROR("SDL_Window was null"); }
    if (!renderer) { return MAKE_ERROR("SDL_Renderer was null"); }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    isInitialized_ = true;

    return Void{};
}

void GuiContext::ProcessEvent(SDL_Event& ev)
{
    if (!isInitialized_) { return; }

    ImGui_ImplSDL2_ProcessEvent(&ev);
}

void GuiContext::RenderPrepare()
{
    if (!isInitialized_) { return; }

    ImGui::Render();
}

void GuiContext::RenderPresent(SDL_Renderer* renderer)
{
    if (!isInitialized_) { return; }

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void GuiContext::NewFrame()
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

#endif
