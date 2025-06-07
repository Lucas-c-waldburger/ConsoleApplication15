#pragma once
#include "custom/CustomEventData.h"




//namespace detail {
//template <SomeCustomEvent...Ts>
//struct is_system_event_group_types 
//{
//    using Types = std::tuple<Ts...>;
//    static constexpr bool value = sizeof...(Ts) == 2 &&
//        std::is_same_v<std::tuple_element_t<0, Types>, events::SystemEventStart> &&
//        std::is_same_v<std::tuple_element_t<1, Types>, events::SystemEventEnd>;
//};
//} // detail
//
//template <SomeCustomEvent...Ts>
//inline constexpr bool is_system_event_group_types_v = detail::is_system_event_group_types<Ts...>::value;


template <SomeCustomEvent...Ts> requires (sizeof...(Ts) >= 1)
class EventGroup
{
private:
    using Types = std::tuple<Ts...>;
    using FrontType = std::tuple_element_t<0, Types>;
    using BackType = std::tuple_element_t<sizeof...(Ts) - 1, Types>;

    EventGroup() = default;

    static inline bool isRegistered_ = false;

public:
    static Result<Void> Register() //requires (!is_system_event_group_types_v<Ts...>)
    {
        if (isRegistered_)
        {
            return Void{};
        }

        if (auto oc = RegisterEvents<Ts...>(); !oc.Success())
        {
            return oc.GetError();
        }

        isRegistered_ = true;

        return Void{};
    }

    static bool IsValid() 
    { 
        return isRegistered_; 
    }

    struct Range
    {
        static uint32_t GetStart() { return (isRegistered_) ? FrontType::GetEventType() : kInvalidEventType; }
        //static constexpr uint32_t GetStart() requires (!is_system_event_group_types_v<Ts...>) { return SDL_FIRSTEVENT; }

        static uint32_t GetEnd() { return (isRegistered_) ? BackType::GetEventType() : kInvalidEventType; }
        //static constexpr uint32_t GetEnd() requires (!is_system_event_group_types_v<Ts...>) { return SDL_USEREVENT - 1; }
    };
};

template <>
class EventGroup<events::SystemEventStart, events::SystemEventEnd>
{
private:
    EventGroup() = default;

public:
    static Result<Void> Register() { return Void{}; }

    static bool IsValid() { return true; }

    struct Range
    {
        static constexpr uint32_t GetStart() { return SDL_FIRSTEVENT; }
        static constexpr uint32_t GetEnd() { return SDL_USEREVENT - 1; }
    };
};


// event group concept
namespace detail {
template <typename>
struct is_event_group : std::false_type {};

template <typename...Ts>
struct is_event_group<EventGroup<Ts...>> : std::true_type {};
} // detail

template <typename T>
concept SomeEventGroup = detail::is_event_group<T>::value && requires {
    { T::Range::GetStart() } -> std::same_as<uint32_t>;
    { T::Range::GetEnd() } -> std::same_as<uint32_t>;
};