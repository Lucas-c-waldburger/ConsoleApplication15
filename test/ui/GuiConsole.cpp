#include "GuiConsole.h"

#if IMGUI_ENABLED
#include "GuiMouse.h"

namespace ui {

void GuiConsole::Init()
{
    if (!sink_)
    {
        sink_ = Logger::AddSink<GuiConsoleSinkSt>();
    }
}

bool GuiConsole::Draw()
{
    //ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);

    bool isOpen = true;

    if (!ImGui::Begin(GetEditorWindowName(EditorWindowType::ConsoleWindow).data(), &isOpen))
    {
        ImGui::End();

        return isOpen;
    }

    GuiMouse::EvaluateInsideWindow(EditorWindowType::ConsoleWindow);

    if (!sink_ || !Logger::Get())
    {
        ImGui::TextUnformatted("Console is not available.");
        ImGui::End();

        return isOpen;
    }

    if (ImGui::Button("Clear"))
    {
        sink_->Clear();
    }

    ImGui::Separator();

    ImGui::BeginChild(
        "LogEntries",
        ImVec2(0.0f, 0.0f),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    GuiMouse::EvaluateInsideWindow(EditorWindowType::ConsoleWindow);

    auto messages = sink_->GetMessages();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(messages.size()));

    const bool wasAtBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& msg = messages[static_cast<size_t>(i)];

            ImGui::PushStyleColor(ImGuiCol_Text, GetLogLevelColor(msg.level));

            ImGui::TextUnformatted(msg.stringContent.c_str());

            ImGui::PopStyleColor();
        }
    }

    if (wasAtBottom)
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();

    return isOpen;
}


} // ui

#endif