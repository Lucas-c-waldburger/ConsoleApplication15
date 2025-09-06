#pragma once
#include "../../inputs/controller/GameControllerInputSource.h"
#include "../../core/Signal.h"
#include "../../core/CatTypeID.h"
#include "../../core/TypeListVectorMap.h"
#include "../../callbacks/EventCallbackRegistry.h"
#include "../../events/data/EventDataIncludes.h"


//template <SomeEventData T>
//using EventSignal = Signal<const T&>;
//
//namespace detail {
//	template <typename TList>
//	struct event_tuple;
//
//	template <typename TList>
//	struct subscriber_list;
//
//	template <SomeEventData...Ts>
//	struct subscriber_list<TypeList<Ts...>> {
//		using type = std::tuple<EventSignal<Ts>...>;
//	};
//}
//
//using SubscriberList = detail::subscriber_list<EventDataTypeList>::type;
//
//
//
//class EventSubscribers
//{
//public:
//	template <SomeEventData T>
//	void Notify(const T& event)
//	{
//		std::get<EventSignal<T>>(subscriberList_).Emit(event);
//	}
//
//	template <typename Fn>
//	SignalToken Connect(Fn&& fn)
//	{
//		return std::get<EventSignal<T>>(subscriberList_).Connect(std::forward<Fn>(fn));
//	}
//
//private:
//	SubscriberList subscriberList_;
//
//	
//};


//struct EventFamily;
//
//template <typename T> requires SomeEventData<T>
//struct TypeInFamily<EventFamily, T> : std::true_type {};
//
//struct EventTypeID : FamilyTypeID<EventFamily> {};
//
//template <typename EventT, typename HandlerT>
//struct ConvertEventCallbackArgs
//{
//	static inline fu2::unique_function<void(const EventT&, HandlerT&)> value =
//		[](const EventT& event, HandlerT& handler) {
//			
//		};
//};
//
//struct Subscriber
//{
//	void (*callback)(void* event, void* instance);
//	void* instance;
//};
//
//struct Dispatcher
//{
//	std::vector<std::vector<Subscriber>> subscribers;
//
//	template <SomeEventData T, typename Fn>
//	void Connect(Fn&& fn) 
//	{
//		size_t id = EventTypeID::value<T>;
//		if (subscribers.size() <= id)
//		{
//			subscribers.resize(EventTypeID::value<T> + 1);
//		}
//
//		subscribers[id].emplace_back([](void*, void* eventPtr) {
//			if (auto event = static_cast<T*>(eventPtr))
//			{
//				std::invoke(fn, event);
//			}			
//		}, nullptr);
//	}
//};  
//
//class EventSignalHub
//{
//public:
//	template <typename Fn> 
//	SignalToken Connect(Fn&& fn)
//	{
// 
//	}
//
//	//void Dispatch(EventDataStorage& eventStorage)
//	//{
//
//	//}
//
//private:
//	template <typename Fn>
//	static auto WrapFn(Fn&& callbackFn)
//	{
//		return [fn = std::forward<Fn>(callbackFn)](const Event& ev) -> void {
//			if (const auto* castEv = EventDataCast<ExtractEventDataTypeFromFnArgs<Fn>>(ev))
//			{
//				return std::invoke(fn, *castEv);
//			}
//		};
//	}
//
//	template <typename Fn> 
//	static auto thing(Fn&& fn)
//	{
//
//	}
//
//	//template <size_t...Is>
//	//static void DispatchImpl2(const TypeListVectorMap<EventDataTypeList>& storage, 
//	//				          const EventSignalTuple& signals)
//	//{
//	//	(())
//	//}
//
//	//template <SomeEventData T>
//	//void DispatchImpl(const std::vector<T>& events)
//	//{
//	//	for (const auto& event : events)
//	//	{
//	//		std::get<EventSignalEmitter<T>>(eventSignals_).Emit(event);
//	//	}
//	//}
//
//	std::vector<EventSignal> eventSignals_;
//};



// Shared map boiler-plate. Templated on key type (with optional hash/eq)
//template <typename Key, typename...Args>
//class IEventCallbacksMap
//{
//public:
//	using MapType = std::unordered_map<Key, std::vector<Handle<EventCallback>>, Args...>;
//
//	auto Erase(typename MapType::iterator it)
//	{
//		return map_.erase(it);
//	}
//
//	template <typename T> requires std::convertible_to<T, Key>
//	bool Contains(T&& key)
//	{
//		return map_.contains(std::forward<T>(key));
//	}
//
//	template <typename T> requires std::convertible_to<T, Key>
//	auto Find(T&& key)
//	{
//		return map_.find(std::forward<T>(key));
//	}
//
//	template <typename T> requires std::convertible_to<T, Key>
//	const auto Find(T&& key) const
//	{
//		return map_.find(std::forward<T>(key));
//	}
//
//	template <typename T> requires std::convertible_to<T, Key>
//	std::vector<Handle<EventCallback>>& GetHandles(T&& key)
//	{
//		return map_[std::forward<T>(key)];
//	}
//
//	auto begin() { return map_.begin(); }
//	auto begin() const { return map_.begin(); }
//	auto end() { return map_.end(); }
//	auto end() const { return map_.end(); }
//
//protected:
//	MapType map_;
//};

//class EventCallbacksMap : 
//	public std::unordered_map<uint32_t, std::vector<Handle<EventCallback>>>
//{
//private:
//	using Super = std::unordered_map<uint32_t, std::vector<Handle<EventCallback>>>;
//
//public:
//	using Super::Super; // Inherit constructors
//
//	// prevent slicing 
//	EventCallbacksMap(const Super&) = delete;
//	EventCallbacksMap& operator=(const Super&) = delete;
//
//	// prevent conversion to base
//	operator Super& () = delete;
//	operator const Super& () const = delete;
//
//	bool AddHandle(const Handle<EventCallback>& handle)
//	{
//		if (!handle.IsValid())
//		{
//			return false;
//		}
//
//		Super::operator[](handle.GetEventType()).push_back(handle);
//
//		return true;
//	}
//
//	bool RemoveHandle(const Handle<EventCallback>& handle)
//	{
//		if (!handle.IsValid())
//		{
//			return false;
//		}
//
//		auto it = Super::find(handle.GetEventType());
//		if (it == Super::end())
//		{
//			return false;
//		}
//
//		return core::Erase(it->second, handle);
//	}
//
//	Handle<EventCallback> FindHandle(const Handle<EventCallback>& handle) const
//	{
//		if (!handle.IsValid())
//		{
//			return {};
//		}
//
//		auto it = Super::find(handle.GetEventType());
//		if (it == Super::end())
//		{
//			return {};
//		}
//
//		auto handleIt = core::Find(it->second, handle);
//
//		return (handleIt != it->second.end()) ? *handleIt : Handle<EventCallback>{};
//	}
//};
//
//class GameControllerInputCallbacksMap : 
//	public std::unordered_map<GameControllerInputSource, std::vector<Handle<EventCallback>>>
//{
//public:
//private:
//	using Super = 
//		std::unordered_map<GameControllerInputSource, std::vector<Handle<EventCallback>>>;
//
//public:
//	using Super::Super; // Inherit constructors
//
//	// prevent slicing 
//	GameControllerInputCallbacksMap(const Super&) = delete;
//	GameControllerInputCallbacksMap& operator=(const Super&) = delete;
//
//	// prevent conversion to base
//	operator Super& () = delete;
//	operator const Super& () const = delete;
//
//	bool AddHandle(GameControllerInputSource src, const Handle<EventCallback>& handle)
//	{
//		if (src == GameControllerInputSource::Invalid)
//		{
//			return false;
//		}
//		if (!handle.IsValid() || 
//			handle.GetEventType() != events::GameControllerInput::eventType)
//		{
//			return false;
//		}
//
//		Super::operator[](src).push_back(handle);
//
//		return true;
//	}
//
//	bool RemoveHandle(GameControllerInputSource src, const Handle<EventCallback>& handle)
//	{
//		if (src == GameControllerInputSource::Invalid)
//		{
//			return false;
//		}
//		if (!handle.IsValid() ||
//			handle.GetEventType() != events::GameControllerInput::eventType)
//		{
//			return false;
//		}
//
//		auto it = Super::find(src);
//		if (it == Super::end())
//		{
//			return false;
//		}
//
//		return core::Erase(it->second, handle);
//	}
//
//	Handle<EventCallback> FindHandle(GameControllerInputSource hint, 
//									 const Handle<EventCallback>& handle)
//	{
//		if (!handle.IsValid())
//		{
//			return {};
//		}
//
//		auto it = Super::find(hint);
//		if (it == Super::end())
//		{
//			return {};
//		}
//
//		auto handleIt = core::Find(it->second, handle);
//
//		return (handleIt != it->second.end()) ? *handleIt : Handle<EventCallback>{};
//	}
//
//	Handle<EventCallback> FindHandle(const Handle<EventCallback>& handle)
//	{
//		if (!handle.IsValid())
//		{
//			return {};
//		}
//
//		for (auto it = Super::begin(); it != Super::end(); ++it)
//		{
//			auto handleIt = core::Find(it->second, handle);
//
//			if (handleIt != it->second.end())
//			{
//				return *handleIt;
//			}
//		}
//
//		return {};
//	}
//};