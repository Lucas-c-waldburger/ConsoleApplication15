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


template <template <typename> class Wrap>
struct AudioSettingsTemplate
{
    Wrap<int> volume;
    Wrap<int> loopCount;
    Wrap<AudioFadeMs> fadeMs;
    AudioSpatialData spatial;

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
               lhs.fadeMs == rhs.fadeMs && lhs.spatial == rhs.spatial;
    }
};



//struct AudioSettings
//{
//    void f()
//    {
//        AudioChannelSettings set;
//        set.volume = 1;
//    }
//
//    struct FadeMs
//    {
//        int in = 0;
//        int out = 0;
//        friend constexpr bool operator==(const FadeMs& lhs, const FadeMs& rhs)
//        {
//            return lhs.in == rhs.in && lhs.out == rhs.out;
//        }
//    };
//
//    int baseVolume = MIX_MAX_VOLUME / 2;
//    int loopCount = 0;
//    FadeMs fadeMs;
//    AudioSpatialData spatial;
//
//    friend constexpr bool operator==(const AudioSettings& lhs, const AudioSettings& rhs)
//    {
//        return lhs.baseVolume == rhs.baseVolume && lhs.loopCount == rhs.loopCount &&
//               lhs.fadeMs == rhs.fadeMs && lhs.spatial == rhs.spatial; 
//    }
//};