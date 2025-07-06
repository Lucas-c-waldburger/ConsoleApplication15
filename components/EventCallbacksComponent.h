#pragma once
#include "BaseComponent.h"
//#include "../events/EventCallbackRegistry.h"
#include "../callbacks/EventCallbackRegistry.h"

struct EventCallbacks : BaseComponent<EventCallbacks>
{ 
	std::unordered_map<uint32_t, Handle<EventCallbackDescriptor>> table;
};

//template <typename Ptr>
//struct member_value_type;
//
//template <typename Class, typename T>
//struct member_value_type<T Class::*> {
//	using type = T;
//};
//
//template <typename Ptr>
//using member_value_type_t = typename member_value_type<Ptr>::type;

//template <auto MemberPtr>
//class CallbackKeyTable
//{
//public:
//	using MapFirstType = member_value_type_t<decltype(MemberPtr)>
//
//	auto AddKey(const EventCallbackRegistry::Key& key)
//	{
//		return table_.emplace(key.*MemberPtr, key);
//	}
//	auto AddKey(EventCallbackRegistry::Key&& key)
//	{
//		return table_.emplace(key.*MemberPtr, std::move(key));
//	}
//
//	EventCallbackRegistry::Key& operator[](const MapFirstType& first)
//	{
//		return table_[first];
//	}
//
//	auto Find(const MapFirstType& first)
//	{
//		return table_.find(first);
//	}
//	auto Find(const MapFirstType& first) const
//	{
//		return table_.find(first);
//	}
//
//private:
//	std::unordered_map<MapFirstType, EventCallbackRegistry::Key> table_;
//};
//
//using EventCallbackKeyTable = CallbackKeyTable<&EventCallbackRegistry::Key::eventType>;


//class EventCallbackKeyTable
//{
//public:
//	using MapType = std::unordered_map<uint32_t, EventCallbackRegistry::Key>;
//	using Iter = MapType::iterator;
//	using CIter = MapType::const_iterator;
//
//	auto AddKey(const EventCallbackRegistry::Key& key) { return table_.emplace(key.eventType, key); }
//	auto AddKey(EventCallbackRegistry::Key&& key) { return table_.emplace(key.eventType, std::move(key)); }
//	
//	EventCallbackRegistry::Key& operator[](uint32_t eventType) { return table_[eventType]; }
//	
//	auto Find(uint32_t eventType) { return table_.find(eventType); }
//	auto Find(uint32_t eventType) const { return table_.find(eventType); }
//
//	bool Erase(uint32_t eventType) { return table_.erase(eventType); }
//	Iter Erase(Iter iter) { return table_.erase(iter); }
//	Iter Erase(CIter iter) { return table_.erase(iter); }
//
//	auto begin() { return table_.begin(); }
//	auto begin() const { return table_.begin(); }
//
//	auto end() { return table_.end(); }
//	auto end() const { return table_.end(); }
//
//private:
//	MapType table_;
//};
// 
//struct EventCallbacks : BaseComponent<EventCallbacks>
//{ 
//	EventCallbackKeyTable table;
//};


