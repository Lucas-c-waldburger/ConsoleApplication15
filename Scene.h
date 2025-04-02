#pragma once
#include <SDL.h>
#include <unordered_map>
#include <typeindex>
#include "ecs/Ecs.h"
#include "core/Monitoring.h"

//template <typename Derived>
//struct CustomEventData
//{
//	template <typename T>
//	friend Result<Void> RegisterCustomEventDataTypes();
//
//	using EventData = Derived;
//	static uint32_t GetEventType() { return eventType; }
//private:
//	static inline uint32_t eventType = static_cast<uint32_t>(-1);
//};
//
//namespace detail {
//template <typename T>
//struct is_custom_event_data_type : std::false_type {};
//
//template <typename T>
//struct is_custom_event_data_type<CustomEventData<T>> : std::true_type {};
//} // detail
//
//template <typename T>
//static constexpr bool is_custom_event_data_type_v = detail::is_custom_event_data_type<T>::value;
//
//template <typename T>
//concept CustomEventDataType = is_custom_event_data_type_v<T>;
//
//
//namespace detail {
//template <typename T>
//struct register_custom_event_data_types;
//
//template <typename...Ts>
//struct register_custom_event_data_types<TypeList<Ts...>> {
//	static Result<Void> Call()
//	{
//		uint32_t newEvType = SDL_RegisterEvents(sizeof...(Ts));
//		if (newEvType == kInvalidEventType)
//		{
//			return MAKE_ERROR("SDL_RegisterEvents failed, no valid event type");
//		}
//
//		((Ts::eventType = newEvType++), ...);
//
//		return Void{};
//	}
//};
//} // detail
//
//template <typename T>
//static Result<Void> RegisterCustomEventDataTypes()
//{
//	return detail::register_custom_event_data_types<T>::Call();
//}

//template <typename T>
//struct register_custom_event_data_types;
//
//template <typename...Ts>
//struct register_custom_event_data_types<TypeList<Ts...>>
//{
//	static Result<Void> Call() 
//	{
//		uint32_t newEvType = SDL_RegisterEvents(sizeof...(Ts));
//		if (newEvType == kInvalidEventType)
//		{
//			return MAKE_ERROR("SDL_RegisterEvents failed, no valid event type");
//		}
//
//
//	}
//};

//class CustomEvents
//{
//public:
//	static constexpr uint32_t kInvalidEventType = static_cast<uint32_t>(-1);
//	using EventDataTypesMap = std::unordered_map<std::type_index, uint32_t>;
//
//	template <typename T>
//	static Result<SDL_Event> MakeNewEvent(T&& evData, Sint32 code = 0)
//	{
//		if (!(SDL_WasInit(0)))
//		{
//			return MAKE_ERROR("SDL not initialized");
//		}
//
//		std::type_index idx = typeid(T);
//		auto& eventDataTypes = GetMap();
//
//		if (!eventDataTypes.contains(idx))
//		{
//			uint32_t newEvType = SDL_RegisterEvents(1);
//			if (newEvType == kInvalidEventType)
//			{
//				return MAKE_ERROR("SDL_RegisterEvents failed, no valid event type");
//			}
//
//			eventDataTypes.emplace(idx, newEvType);
//		}
//
//		SDL_Event ev{
//			.type = eventDataTypes[idx],
//			.user.code = code,
//			.user.data1 = new T{ std::forward<T>(eventData) },
//			.user.data2 = nullptr
//		};
//
//		return ev;
//	}
//
//	template <typename T>
//	Result<Void> FreeEvent(SDL_Event& ev)
//	{
//		if (ev.type < SDL_USEREVENT)
//		{
//			LOG_WARNING("Event passed was not a user event");
//
//			return Void{};
//		}
//		if (!ev.user.data1)
//		{
//			return Void{};
//		}
//
//		auto& eventDataTypes = GetMap();
//
//		auto it = eventDataTypes.find(typeid(T));
//		if (it == eventDataTypes.end())
//		{
//			return MAKE_ERROR("Event data type was not registered");
//		}
//
//		if (it->second != ev.type)
//		{
//			return MAKE_ERROR("Event data type does not match its mapped uint32_t");
//		}
//
//		delete static_cast<T*>(ev.user.data1);
//		ev.user.data1 = nullptr;
//
//		return Void{};
//	}
//
//	template <typename T>
//	static bool IsEventData(const SDL_Event& ev) 
//	{ 
//		auto& eventDataTypes = GetMap();
//
//		auto it = eventDataTypes.find(typeid(T)); 
//
//		return it != eventDataTypes.end() && ev.type == it->second;
//	}
//
//	template <typename T>
//	T* GetEventData(SDL_Event& ev)
//	{
//		if (ev.type < SDL_USEREVENT)
//		{
//			LOG_WARNING("Event passed was not a user event");
//			return nullptr;
//		}
//		if (!ev.user.data1)
//		{
//			return nullptr;
//		}
//
//		auto& eventDataTypes = GetMap();
//
//		auto it = eventDataTypes.find(typeid(T));
//		if (it == eventDataTypes.end())
//		{
//			LOG_ERROR("Event data type was not registered");
//			return nullptr;
//		}
//
//		if (it->second != ev.type)
//		{
//			LOG_ERROR("Event data type does not match its mapped uint32_t");
//			return nullptr;
//		}
//
//		return static_cast<T*>(ev.user.data1);
//	}
//
//	template <typename T>
//	uint32_t GetEventType()
//	{
//		std::type_index idx = typeid(T);
//		auto& eventDataTypes = GetMap();
//
//		if (!eventDataTypes.contains(idx))
//		{
//			uint32_t newEvType = SDL_RegisterEvents(1);
//			if (newEvType == kInvalidEventType)
//			{
//				LOG_ERROR("SDL_RegisterEvents failed, no valid event type");
//				return kInvalidEventType;
//			}
//
//			eventDataTypes.emplace(idx, newEvType);
//		}
//		
//		return eventDataTypes[idx];
//	}
//	
//private:
//	static EventDataTypesMap& GetMap()
//	{
//		static std::unique_ptr<EventDataTypesMap> eventDataTypes;
//		if (!eventDataTypes)
//		{
//			eventDataTypes = std::make_unique<EventDataTypesMap>();
//		}
//
//		return *eventDataTypes;
//	}
//};

//template <uint32_t evType, typename EventDataT> 
//struct CustomEvent
//{
//	using EventDataType = EventDataT;
//	static constexpr uint32_t eventType = evType;
//
//	static Result<SDL_Event> MakeNew(EventDataT&& eventData)
//	{
//		if (eventType == static_cast<uint32_t>(-1))
//		{
//			return MAKE_ERROR("Invalid event type! was this event registered?");
//		}
//
//		SDL_Event ev{
//			.type = eventType,
//			.user.code = 0,
//			.user.data1 = new EventDataT{ std::forward<EventDataT>(eventData) },
//			.user.data2 = nullptr
//		};
//
//		return ev;
//	}
//
//private:
//	//template <typename T>
//	//static void FillUserData(SDL_Event& ev, T&& eventData)
//	//{
//	//	if (!ev.user.data1)
//	//	{
//	//		ev.user.data1 = new T{ std::forward<T>(eventData) };
//	//		assert(ev.user.data1);
//	//	}
//	//	else if (!ev.user.data2)
//	//	{
//	//		ev.user.data2 = new T{ std::forward<T>(eventData) };
//	//		assert(ev.user.data2);
//	//	}
//	//	assert(0 && "wtf how did we get here");
//	//}
//};