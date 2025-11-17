#include "ImguiContext.h"
#include "ImguiUtils.h"

void ImguiContext::Exit()
{
    if (!isInitialized_) { return; }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

Result<Void> ImguiContext::Init(SDL_Window* window, SDL_Renderer* renderer)
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

void ImguiContext::ProcessEvent(SDL_Event& ev)
{
    if (!isInitialized_) { return; }

    ImGui_ImplSDL2_ProcessEvent(&ev);
}

void ImguiContext::RenderPrepare()
{
    if (!isInitialized_) { return; }

    ImGui::Render();
}

void ImguiContext::RenderPresent(SDL_Renderer* renderer)
{
    if (!isInitialized_) { return; }

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

//// IMGUIWINDOW
//ImguiWindow::ImguiWindow(const char* title, bool* pOpen, ImGuiWindowFlags flags)
//{
//    ImGui::Begin(title, pOpen, flags);
//}
//
//ImguiWindow::~ImguiWindow()
//{
//    ImGui::End();
//}
