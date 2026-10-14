#include "GuiConsole.h"

#if IMGUI_ENABLED
#include "GuiMouse.h"
#include <imgui_internal.h>

namespace ui {

struct ConsoleLine
{
    std::string_view text;
	spdlog::level::level_enum level;
};

void GuiConsole::Init()
{
    if (!sink_)
    {
        sink_ = Logger::AddSink<GuiConsoleSinkSt>();
    }
}

bool GuiConsole::Draw()
{
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
    auto lines = sink_->GetConsoleLines();

    const bool hasNewMessages = messages.size() > lastMessageCount_;
    const bool wasAtBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(lines.size()));

    if (hasNewMessages)
    {
        clipper.IncludeItemsByIndex(lines.size() - 1, lines.size());
	}

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& line = lines[static_cast<size_t>(i)];
			assert(line.messageIndex < messages.size());
            const auto& msg = messages[line.messageIndex];

            const char* begin = msg.stringContent.data() + line.start;

            ImGui::PushStyleColor(ImGuiCol_Text, GetLogLevelColor(msg.level));

            ImGui::TextUnformatted(begin, begin + line.length);

            ImGui::PopStyleColor();
        }
    }

    if (hasNewMessages)
    {
        ImGui::SetScrollY(999999.0f);
    }

	lastMessageCount_ = messages.size();

    ImGui::EndChild();
    ImGui::End();

    return isOpen;
}


} // ui

#endif