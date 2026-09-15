#include "MusicVisualizer.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <algorithm>

namespace ui {

void MusicVisualizer::Start()
{
    Mix_SetPostMix(&MusicVisualizer::PostMix, this);
}

void MusicVisualizer::Stop()
{
    Mix_SetPostMix(nullptr, nullptr);
}

void MusicVisualizer::Reset()
{
    for (auto& sample : samples_)
    {
        sample.store(0.0f, std::memory_order_relaxed);
    }

    writeIndex_.store(0, std::memory_order_relaxed);
}

//float MusicVisualizer::GetLevel() const
//{
//    return level_.load(std::memory_order_relaxed);
//}

void MusicVisualizer::DrawWaveform()
{
    constexpr float width = 300.0f;
    constexpr float height = 80.0f;

    ImVec2 min = ImGui::GetCursorScreenPos();
    min.x += std::max(
        (ImGui::GetContentRegionAvail().x - width) * 0.5f,
        0.0f);

    const ImVec2 max = {
        min.x + width,
        min.y + height
    };

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    std::array<ImVec2, kSampleCount> points;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const float sample = GetSample(i);

        const float x = min.x + static_cast<float>(i) / (points.size() - 1) * width;

        const float y = min.y + height * 0.5f - sample * height * 0.5f;

        points[i] = { x, y };
    }

    drawList->AddPolyline(
        points.data(),
        static_cast<int>(points.size()),
        IM_COL32(100, 180, 255, 255),
        ImDrawFlags_None,
        2.0f);

    ImGui::Dummy({ width, height });
}

void SDLCALL MusicVisualizer::PostMix(void* userdata, Uint8* stream, int len)
{
    auto* self = static_cast<MusicVisualizer*>(userdata);
    if (!self)
    {
        return;
    }

    const auto* samples = reinterpret_cast<const Sint16*>(stream);

    const size_t sampleCount = len / sizeof(Sint16);

    self->PushSamples(samples, sampleCount);

    //auto& self = *static_cast<MusicVisualizer*>(userdata);

    //const auto* samples = reinterpret_cast<const Sint16*>(stream);

    //const size_t count = len / sizeof(Sint16);

    //Sint16 peak = 0;

    //for (size_t i = 0; i < count; ++i)
    //{
    //    peak = std::max(
    //        peak,
    //        static_cast<Sint16>(std::abs(samples[i])));
    //}

    //self.level_.store(
    //    static_cast<float>(peak) / 32768.0f,
    //    std::memory_order_relaxed);
}

void MusicVisualizer::PushSamples(const Sint16* samples, size_t count)
{
    for (size_t i = 0; i < count; i += 2) // += 2 since alternates L R L R in stereo
    {
        // Stereo -> mono.
        const float left = samples[i] / 32768.0f;
        const float right = samples[i + 1] / 32768.0f;

        const float mono = (left + right) * 0.5f;

        const size_t index =
            writeIndex_.fetch_add(1, std::memory_order_relaxed)
            % kSampleCount;

        samples_[index].store(mono, std::memory_order_relaxed);
    }
}

float MusicVisualizer::GetSample(size_t idx) const
{
    const size_t write = writeIndex_.load(std::memory_order_relaxed);

    const size_t bufferIndex = (write + idx) % kSampleCount;

    return samples_[bufferIndex].load(std::memory_order_relaxed);
}

} // ui

#endif