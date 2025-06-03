#pragma once
#include "../core/Handle.h"
#include "custom/CustomEventData.h"

struct EventCallback;

template <>
class Handle<EventCallback> : public HandleBase<Handle<EventCallback>>
{
public:
    friend class HandleBase<Handle<EventCallback>>;
    friend class EventCallbackMap;

    Handle() = default;
    bool operator==(const Handle& rhs) const
    {
        return id_ == rhs.id_ && eventType_ == rhs.eventType_;
    }

private:
    Handle(int id, uint32_t evType) : id_(id), eventType_(evType) {}

    bool IsValidImpl() const
    {
        return id_ < idCount && eventType_ >= SDL_USEREVENT && 
                                eventType_ < kInvalidEventType;
    }

    size_t GetHashImpl() const noexcept
    {
        std::size_t hash = 0;
        HashCombine(hash, std::hash<int>{}(id_));
        HashCombine(hash, std::hash<uint32_t>{}(eventType_));

        return hash;
    }

    template <SomeCustomEvent T>
    static Handle CreateImpl()
    {
        return { idCount++, T::GetEventType() };
    }

    static inline int idCount = 0;

    int id_ = -1;
    uint32_t eventType_ = kInvalidEventType;
};