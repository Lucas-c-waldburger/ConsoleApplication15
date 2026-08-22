#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <vector>
#include <string>
#include <format>
#include <span>
#include "../../core/Logger.h"

namespace ui {

struct GuiLogMessage
{
    spdlog::level::level_enum level = spdlog::level::level_enum::off;
    std::string stringContent;
};

template <typename Mutex>
class GuiConsoleSink : public spdlog::sinks::base_sink<Mutex>
{
public:
    std::span<const GuiLogMessage> GetMessages() const
    {
        return messages_;
    }

    void Clear() requires (!std::same_as<Mutex, spdlog::details::null_mutex>)
    {
        std::scoped_lock lock(this->mutex_);
        messages_.clear();
    }

    void Clear()
    {
        messages_.clear();
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        messages_.emplace_back(msg.level, fmt::to_string(formatted));
    }

    void flush_() override {}

private:
    std::vector<GuiLogMessage> messages_;
};

using GuiConsoleSinkMt = GuiConsoleSink<std::mutex>;
using GuiConsoleSinkSt = GuiConsoleSink<spdlog::details::null_mutex>;

class GuiConsole
{
public:
    static void Init();

    static bool Draw();


private:
    static ImVec4 GetLogLevelColor(spdlog::level::level_enum lvl)
    {
        using Lvl = spdlog::level::level_enum;
        switch (lvl)
        {
        case Lvl::debug:
            return ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        case Lvl::info:
            return ImVec4(0.3f, 0.8f, 1.0f, 1.0f);
        case Lvl::warn:
            return ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        case Lvl::err:
            return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        case Lvl::critical:
            return ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
        }

        return ImGui::GetStyleColorVec4(ImGuiCol_Text);
    }
    
    static inline std::shared_ptr<GuiConsoleSinkSt> sink_;
};

} // ui

#endif