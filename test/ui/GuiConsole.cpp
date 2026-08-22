#include "GuiConsole.h"

#if IMGUI_ENABLED

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
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);

    bool isOpen = true;

    if (!ImGui::Begin("Console", &isOpen))
    {
        ImGui::End();

        return isOpen;
    }

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

    auto messages = sink_->GetMessages();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(messages.size()));

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

    ImGui::EndChild();
    ImGui::End();

    return isOpen;
}


} // ui

#endif