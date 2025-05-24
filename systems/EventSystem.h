#pragma once
#include "../deps/function2/function2.hpp"
#include "../events/handler/GameControllerEventHandler.h"
#include "../events/handler/MouseEventHandler.h"
#include "../events/EventBuffer.h"
#include "../events/custom/CustomEvents.h"
#include "../events/custom/CustomEventDataRegistry.h"
#include "../core/Monitoring.h"
#include "System.h"

class EventSystem : public System
{
public:
	bool Poll(SDL_Event& ev);
	void DistributeEvents();

	template <typename T> T& GetHandler();
	template <> GameControllerEventHandler& GetHandler<GameControllerEventHandler>();

private:
	EventBuffer eventBuffer_;
	GameControllerEventHandler gameControllerHandler_;
};

template <>
inline GameControllerEventHandler& EventSystem::GetHandler()
{
	return gameControllerHandler_;
}


//// EVENT DOMAIN
//class EventDomain 
//{
//public:
//    struct Collision;
//    struct GameController;
//private:
//    EventDomain() = default;
//};
//
//#define EVENT_DOMAIN_REGISTRY \
//    EventDomain::Collision, \
//    EventDomain::GameController \
//
//template <typename Derived> requires type_in_pack_v<Derived, EVENT_DOMAIN_REGISTRY>
//struct EventDomainWrapper 
//{
//    static constexpr size_t domainIndex = index_of_v<Derived, TypeList<EVENT_DOMAIN_REGISTRY>>;
//};
//
//template <typename T>
//concept SomeEventDomain = std::derived_from<T, EventDomainWrapper<T>>;
//
//struct EventDomain::Collision : EventDomainWrapper<Collision> {};
//struct EventDomain::GameController : EventDomainWrapper<GameController> {};

// EVENT TYPE
class EventType 
{
public:
    class GameController
    {
    public:
        struct Connected;
        struct Disconnected;

        struct ButtonPressed;
        struct ButtonReleased;

        struct AxisLeftDeadzone;
        struct AxisEnteredDeadzone;

    private:
        GameController() = default;
    };

    class Collision
    {
    public:        
        struct ContactBegin;
        struct ContactEnd;
        struct SensorBegin;
        struct SensorEnd;
        struct Hit;

    private:
        Collision() = default;
    };

    class EntityAction
    {
    public:
        struct Created;
        struct Destroyed;
        struct PositionChanged;

    private:
        EntityAction() = default;
    };
   
private:
    EventType() = default;
};

#define EVENT_TYPE_REGISTRY \
    EventType::GameController::Connected, \
    EventType::GameController::Disconnected, \
    EventType::Collision::ContactBegin, \
    EventType::Collision::ContactEnd, \
    EventType::Collision::SensorBegin, \
    EventType::Collision::SensorEnd, \
    EventType::Collision::Hit

template <typename Derived> requires type_in_pack_v<Derived, EVENT_TYPE_REGISTRY>
struct EventTypeWrapper 
{
    static constexpr size_t eventIndex = index_of_v<Derived, TypeList<EVENT_TYPE_REGISTRY>>;
};

template <typename T>
concept SomeEventType = std::derived_from<T, EventTypeWrapper<T>>;

struct EventType::GameController::Connected : EventTypeWrapper<Connected> 
{
    SDL_JoystickID joystickID = -1;
};
struct EventType::GameController::Disconnected : EventTypeWrapper<Disconnected> 
{
    SDL_JoystickID joystickID = -1;
};
struct EventType::GameController::ButtonPressed : EventTypeWrapper<ButtonPressed>
{
    SDL_JoystickID joystickID = -1;
    SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
};
struct EventType::GameController::ButtonReleased : EventTypeWrapper<ButtonReleased>
{
    SDL_JoystickID joystickID = -1;
    SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
};
struct EventType::GameController::AxisEnteredDeadzone : EventTypeWrapper<AxisEnteredDeadzone>
{
    SDL_JoystickID joystickID = -1;
    SDL_GameControllerAxis axis = SDL_CONTROLLER_AXIS_INVALID;
};
struct EventType::GameController::AxisLeftDeadzone : EventTypeWrapper<ButtonReleased>
{
    SDL_JoystickID joystickID = -1;
    SDL_GameControllerAxis axis = SDL_CONTROLLER_AXIS_INVALID;
};

struct EventType::Collision::ContactBegin : EventTypeWrapper<ContactBegin> 
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};
struct EventType::Collision::ContactEnd : EventTypeWrapper<ContactEnd>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};
struct EventType::Collision::SensorBegin : EventTypeWrapper<SensorBegin>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};
struct EventType::Collision::SensorEnd : EventTypeWrapper<SensorEnd>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};
struct EventType::Collision::Hit : EventTypeWrapper<Hit>
{
    Entity_t entityA = kInvalidEntity;
    Entity_t entityB = kInvalidEntity;
};

struct EventType::EntityAction::Created : EventTypeWrapper<Created>
{
    Entity_t entity;
};
struct EventType::EntityAction::Destroyed : EventTypeWrapper<Destroyed>
{
    Entity_t entity;
};

constexpr size_t kInvalidEventIndex = std::numeric_limits<size_t>::max();

struct Event 
{
    size_t eventIndex = kInvalidEventIndex;
    uint64_t timestamp = 0;
    const void* data = nullptr;
};

template <SomeEventType EvData>
Event MakeEvent(const EvData& evData, uint64_t tStamp)
{
    return Event{
        .eventIndex = EvData::eventIndex,
        .timestamp = tStamp,
        .data = static_cast<const void*>(&evData)
    };
}

template <SomeEventType EvData>
Event MakeEvent(const EvData& evData)
{
    return MakeEvent<T>(evData, SDL_GetTicks64());
}

template <SomeEventType T>
bool IsEventType(const Event& ev) 
{
    return ev.eventIndex == T::eventIndex;
}

template <SomeEventType T>
const T* CastEventData(const Event& ev) 
{
    if (IsEventType<T>(ev))
    {
        return static_cast<const T*>(ev.data);
    }

    return nullptr;
}


struct EventCallback 
{
    fu2::unique_function<ReturnSignal(const Event& ev)> onEvent;
    Handle<EventCallback> handle;
};

template <>
class Handle<EventCallback> : public HandleBase<Handle<EventCallback>>
{
public:
    friend class HandleBase<Handle<EventCallback>>;
    friend class EventCallbackMap;

    Handle() = default;
    bool operator==(const Handle& rhs) const 
    {
        return id_ == rhs.id_ && eventIndex_ == rhs.eventIndex_;
    }

private:
    Handle(int id, size_t evIdx) : id_(id), eventIndex_(evIdx) {}

    bool IsValidImpl() const
    {
        return id_ < idCount && eventIndex_ < TypeList<EVENT_TYPE_REGISTRY>::size;
    }

    size_t GetHashImpl() const noexcept
    {
        std::size_t hash = 0;
        HashCombine(hash, std::hash<int>{}(id_));
        HashCombine(hash, std::hash<size_t>{}(eventIndex_));

        return hash;
    }

    template <SomeEventType T>
    static Handle CreateImpl(T&&)
    {
        return { idCount++, T::eventIndex };
    }

    static inline int idCount = 0;

    int id_ = -1;
    size_t eventIndex_ = kInvalidEventIndex;
};

class EventCallbackMap
{
public:
    template <SomeEventType T, typename Fn>
    Handle<EventCallback> Insert(Fn&& fn)
    {
        auto it = indexMapper_.find(T::eventIndex);
        if (it == indexMapper_.end())
        {
            indexMapper_[T::eventIndex] = callbacks_.size();
            callbacks_.emplace_back();
        }

        auto& cbs = callbacks_[indexMapper_[T::eventIndex]];

        auto handle = Handle<EventCallback>::Create<T>();

        cbs.emplace_back(std::forward<Fn>(fn), handle);

        return handle;
    }

    bool Erase(const Handle<EventCallback>& handle)
    {
        if (!handle.IsValid())
        {
            return false;
        }

        auto it = indexMapper_.find(handle.eventIndex_);
        if (it == indexMapper_.end())
        {
            return false;
        }
        
        assert(it->second < callbacks_.size());

        auto& cbsForEventType = callbacks_[it->second];

        return EraseIf(cbsForEventType, [&handle](const auto& cb) {
            return handle == cb.handle;
        });
    }

    bool HasCallbacksForEventType(size_t evIndex) const
    {
        auto it = indexMapper_.find(evIndex);

        return (it != indexMapper_.end()) ? !callbacks_[it->second].empty() : false;
    }

    template <SomeEventType T>
    bool HasCallbacksForEventType() const
    {
        return HasCallbacksForEventType(T::eventIndex);
    }

    std::vector<EventCallback>& GetCallbacksForEventType(size_t evIndex)
    {
        auto it = indexMapper_.find(evIndex);
        if (it == indexMapper_.end())
        {
            indexMapper_[evIndex] = callbacks_.size();
            callbacks_.emplace_back();
        }

        return callbacks_[indexMapper_[evIndex]];
    }

    template <SomeEventType T>
    std::vector<EventCallback>& GetCallbacksForEventType()
    {
        return GetCallbacksForEventType(T::eventIndex);
    }

private:
    std::vector<std::vector<EventCallback>> callbacks_;
    std::unordered_map<size_t, size_t> indexMapper_;
};

struct EventCallbacks : BaseComponent<EventCallbacks, 14>
{
    EventCallbackMap map;
};

class EventRecord
{
public:
    struct Record
    {
        uint64_t timeStamp = 0;
    };

private:
};

template <SomeEventType...Ts> requires pack_types_unique_v<Ts...>
class EventSystemTemplate
{
public:
    template <PackMemberType<Ts...> T>
    void PostEvent(T&& evData) 
    {
        GetEntry<T>().push_back(std::forward<T>(evData));
    }

    template <PackMemberType<Ts...>...EvTs>
    void DistributeEvents()
    {
        auto entities = GetEntitiesWithEventCallbacks<EvTs...>();
        if (entities.empty())
        {
            return;
        }

        ((DistributeEventsImpl<Ts>(entities)), ...);
    }


private:
    template <SomeEventType...Ts>
    std::vector<Entity> GetEntitiesWithEventCallbacks()
    {
        return ECS::GetAllEntitiesWith<EventCallbacks>([](const EventCallbacks& cbs) {
            return (cbs.map.HasCallbacksForEventType<Ts> || ...);
        });
    }

    template <PackMemberType<Ts...> T>
    void DistributeEventsImpl(std::vector<Entity>& entities)
    {
        auto& entry = GetEntry<T>();

        if (entry.empty())
        {
            return;
        }

        for (const auto& evData : entry)
        {
            auto ev = MakeEvent<T>(evData);
            assert(ev.data != nullptr);

            for (auto& entity : entities)
            {
                assert(entity.IsValid());

                auto& callbackMap = entity.GetComponent<EventCallbacks>().map;
                auto& cbsForEvent = callbackMap.GetCallbacksForEvent<T>();

                auto it = cbsForEvent.begin();
                while (it != cbsForEvent.end())
                {
                    if (it->onEvent)
                    {
                        assert(it->handle.IsValid());

                        auto ret = it->onEvent(ev);
                        if (ret == ReturnSignal::KeepObserving)
                        {
                            ++it;
                            continue;
                        }
                    }

                    it = cbsForEvent.erase(it);
                }
            }
        }

        entry.clear();
    }

    template <PackMemberType<Ts...> T>
    std::vector<T>& GetEntry() {
        return std::get<std::vector<T>>(eventDatas_);
    }
    template <PackMemberType<Ts...> T>
    const std::vector<T>& GetEntry() {
        return std::get<std::vector<T>>(eventDatas_);
    } 

    std::tuple<std::vector<Ts>...> eventDatas_;
};

namespace impl {
    using EventSystem2 = EventSystemTemplate<EVENT_TYPE_REGISTRY>;
}

//class EventCallbackMap
//{
//public:
//    using MapType = std::unordered_map<size_t, std::vector<EventCallback>>;
//    using Iterator = MapType::iterator;
//
//    template <SomeEventType T, typename Fn>
//    Handle<EventCallback> Insert(Fn&& fn)
//    {
//        auto handle = Handle<EventCallback>::Create();
//
//        map_[T::eventIndex].emplace_back(std::forward<Fn>(fn), handle);
//
//        return handle;
//    }
//
//    bool Erase(const Handle<EventCallback>& handle)
//    {
//        auto it = map_.find(handle.eventIndex_);
//        if (it == map_.end())
//        {
//            return false;
//        }
//
//        return EraseIf(it->second, [&handle](const auto& cb) {
//            return handle == cb.handle;
//            });
//    }
//
//
//
//    std::vector<EventCallback>& operator[](size_t evIndex)
//    {
//        return map_[evIndex];
//    }
//
//
//private:
//    MapType map_;
//};
//
//template <SomeEventDomain Dom, SomeEventType...Ts>
//struct filter_by_domain;

//template <SomeEventDomain Dom, SomeEventType T, SomeEventType...Ts>
//struct filter_by_domain<Dom, T, Ts...>
//{
//    using type = std::conditional_t<
//        std::same_as<T::Domain, Dom>, 
//
//};