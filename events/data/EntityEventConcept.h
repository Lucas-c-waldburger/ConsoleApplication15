#pragma once
#include "../EventConcepts.h"
#include "../../ecs/EntityT.h"

namespace detail {

template <size_t I>
struct EntityParticipantSlot { Entity_t entityId; };

template <size_t...Is>
struct EntityParticipantSlots : EntityParticipantSlot<Is>... {};

template <size_t...Is>
struct EntityParticipantsImpl : EntityParticipantSlots<Is...>
{
    template <size_t I> requires ((I == Is) || ...)
    Entity_t& entity()
    {
        return static_cast<EntityParticipantSlot<I>&>(*this).entityId;
    }

    template <size_t I> requires ((I == Is) || ...)
    const Entity_t& entity() const
    {
        return static_cast<const EntityParticipantSlot<I>&>(*this).entityId;
    }
};

template <typename Seq>
struct EntityParticipantsFromSeq;

template <size_t... Is>
struct EntityParticipantsFromSeq<std::index_sequence<Is...>> : 
    EntityParticipantsImpl<Is...> {};

} // detail

template <size_t Count> requires (Count > 0)
struct EntityParticipants : 
    detail::EntityParticipantsFromSeq<std::make_index_sequence<Count>>
{
    static constexpr size_t entityCount = Count;
};

// Inheriting event gets registered as an IEventData
// Count argument gives N different entities as members of the inheriting event
// ex: struct TestEvent : EntityEvent<TestEvent, 2> -> entity<0>(), entity<1>()
template <typename Derived, size_t Count> requires (Count > 0)
struct EntityParticpantEvent : IEventData<Derived>, EntityParticipants<Count>
{};

template <typename T>
concept HasEntityParticipants = requires(T& t, const T& ct) {
    { t.template entity<0>() } -> std::same_as<Entity_t&>;
    { ct.template entity<0>() } -> std::same_as<const Entity_t&>;
    { T::entityCount } -> std::convertible_to<size_t>;
};

template <typename T>
concept SomeEntityParticipantEvent = SomeEventData<T> && HasEntityParticipants<T>;