#pragma once
#include <SDL_mixer.h>
#include "AudioCommon.h"

namespace detail {
template <template <typename> class T, template <typename> class U>
struct is_same_template : std::false_type {};

template <template <typename> class T>
struct is_same_template<T, T> : std::true_type {};
} // detail

template <template <typename> class T, template <typename> class U>
inline constexpr bool is_same_template_v = detail::is_same_template<T, U>::value;

struct AudioFadeMs
{
    int in = 0;
    int out = 0;
    friend constexpr bool operator==(const AudioFadeMs& lhs, const AudioFadeMs& rhs)
    {
        return lhs.in == rhs.in && lhs.out == rhs.out;
    }
};

template <template <typename> class Wrap>
struct AudioSettingsTemplate;

using AudioChannelSettings = AudioSettingsTemplate<std::type_identity_t>;
using AudioUpdateSettings = AudioSettingsTemplate<std::optional>;

namespace detail {
template <template <typename> class Wrap, int val>
struct int_val_or_nullopt;

template <int val>
struct int_val_or_nullopt<std::optional, val>
{
    static constexpr std::optional<int> value = std::nullopt;
};
template <int val>
struct int_val_or_nullopt<std::type_identity_t, val>
{
    static constexpr int value = val;
};
} // detail

template <template <typename> class Wrap, int val>
static constexpr auto int_val_or_nullopt_v = detail::int_val_or_nullopt<Wrap, val>::value;


//// TODO: Fix so that you can do designated initializer construction and still have volume 
//// default to MIN_MAX_VOLUME/2 instead of 0 if Wrap == type_identity_t
template <template <typename> class Wrap>
struct AudioSettingsTemplate
{
    Wrap<int> volume = int_val_or_nullopt_v<Wrap, MIX_MAX_VOLUME / 2>;
    Wrap<int> loopCount = int_val_or_nullopt_v<Wrap, 0>;
    Wrap<AudioFadeMs> fadeMs;
    AudioSpatialData spatial;
    float trackPosition = 0.0f;

    static AudioChannelSettings Default() 
        requires is_same_template_v<Wrap, std::type_identity_t>
    {
        return AudioChannelSettings{
            .volume = MIX_MAX_VOLUME / 2,
            .loopCount = 0 
        };
    }

    static AudioUpdateSettings Default() 
        requires is_same_template_v<Wrap, std::optional>
    {
        return AudioUpdateSettings{};
    }

    friend bool operator==(const AudioSettingsTemplate& lhs, 
                           const AudioSettingsTemplate& rhs)
    {
        return lhs.volume == rhs.volume && lhs.loopCount == rhs.loopCount &&
               lhs.fadeMs == rhs.fadeMs && lhs.spatial == rhs.spatial &&
               lhs.trackPosition == rhs.trackPosition;
    }
};