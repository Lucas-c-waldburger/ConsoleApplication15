#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <SDL_mixer.h>
#include <atomic>
#include <array>

namespace ui {

class MusicVisualizer
{
public:
    static constexpr size_t kSampleCount = 2048;

    void Start();
    void Stop();
    void Reset();
    //float GetLevel() const;
    void DrawWaveform();

private:
    static void SDLCALL PostMix(void* userdata, Uint8* stream, int len);

    void PushSamples(const Sint16* samples, size_t count);
    float GetSample(size_t idx) const;

    //std::atomic<float> level_{ 0.0f };
    std::array<std::atomic<float>, kSampleCount> samples_{};
    std::atomic<size_t> writeIndex_{};
};

#endif




} // ui