#pragma once
#include "AudioEnums.h"
#include "AudioPtrs.h"
#include "../core/commonObjects.h"
#include <SDL_mixer.h>
#include <memory>
#include <string>

template <typename T>
concept SomeMixType = std::same_as<T, Mix_Chunk> ||
                      std::same_as<T, Mix_Music>;

template <template <typename> class Wrap>
struct AudioSpatialDataTemplate;

using RawAudioSpatialData = AudioSpatialDataTemplate<std::type_identity_t>;
using AudioSpatialData = AudioSpatialDataTemplate<std::optional>;

namespace detail {
template <template <typename> class, int16_t>
struct int16_val_or_nullopt;

template <int16_t val>
struct int16_val_or_nullopt<std::optional, val>
{
    static constexpr std::optional<int16_t> value = std::nullopt;
};
template <int16_t val>
struct int16_val_or_nullopt<std::type_identity_t, val>
{
    static constexpr int16_t value = val;
};

template <template <typename> class, uint8_t>
struct uint8_val_or_nullopt;

template <uint8_t val>
struct uint8_val_or_nullopt<std::optional, val>
{
    static constexpr std::optional<uint8_t> value = std::nullopt;
};
template <uint8_t val>
struct uint8_val_or_nullopt<std::type_identity_t, val>
{
    static constexpr uint8_t value = val;
};

template <template <typename> class, uint8_t, uint8_t>
struct panning_vals_or_nullopt;

template <uint8_t l, uint8_t r>
struct panning_vals_or_nullopt<std::optional, l, r>
{
    static constexpr std::optional<HandedPair<uint8_t>> value = std::nullopt;
};
template <uint8_t l, uint8_t r>
struct panning_vals_or_nullopt<std::type_identity_t, l, r>
{
    static constexpr HandedPair<uint8_t> value = HandedPair<uint8_t>{ l, r };
};

} // detail

template <template <typename> class Wrap>
struct AudioSpatialDataTemplate
{
    using Panning = HandedPair<uint8_t>;

    Wrap<int16_t> angle = detail::int16_val_or_nullopt<Wrap, 0>::value;
    Wrap<uint8_t> distance = detail::uint8_val_or_nullopt<Wrap, 0>::value;
    Wrap<Panning> panning = detail::panning_vals_or_nullopt<Wrap, 127, 127>::value;

    friend constexpr bool operator==(const AudioSpatialDataTemplate& lhs,
                                     const AudioSpatialDataTemplate& rhs)
    {
        return lhs.angle == rhs.angle && lhs.distance == rhs.distance &&
               lhs.panning == rhs.panning;
    }
}; 