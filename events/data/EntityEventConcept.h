#pragma once
#include "../EventConcepts.h"
#include "../../ecs/EntityT.h"

namespace detail {

template <size_t I>
struct EntityEventSlot { Entity_t value; };

template <size_t...Is>
struct EntityEventSlots : EntityEventSlot<Is>... {};

template <size_t...Is>
struct EntityEventImpl : EntityEventSlots<Is...>
{
    template <size_t I> requires ((I == Is) || ...)
    Entity_t& entity()
    {
        return static_cast<EntityEventSlot<I>&>(*this).value;
    }

    template <size_t I> requires ((I == Is) || ...)
    const Entity_t& entity() const
    {
        return static_cast<const EntityEventSlot<I>&>(*this).value;
    }
};

template <typename Seq>
struct EntityEventFromSeq;

template <size_t... Is>
struct EntityEventFromSeq<std::index_sequence<Is...>> : EntityEventImpl<Is...> {};

} // detail


// Inheriting event gets registered as an IEventData
// Count argument gives N different entities as members of the inheriting event
// ex: struct TestEvent : EntityEvent<TestEvent, 2> -> entity<0>(), entity<1>()
template <typename Derived, size_t Count> requires (Count > 0)
struct EntityEvent : IEventData<Derived>, 
                     detail::EntityEventFromSeq<std::make_index_sequence<Count>>
{
    static constexpr size_t entityCount = Count;
};

template <typename T>
concept SomeEntityEvent = requires(T t) {
    SomeEventData<T>;
    { t.template entity<0>() } -> std::same_as<Entity_t&>;
    { const_cast<const T&>(t).template entity<0>() } -> std::same_as<const Entity_t&>;
    { T::entityCount } -> std::convertible_to<size_t>;
};