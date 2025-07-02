#pragma once
#include "BaseComponent.h"
#include "../callbacks/EventCallbackRegistry.h"
#include "../inputs/controller/GameControllerInputSource.h"

struct GameControllerInputCallbacks : public BaseComponent<GameControllerInputCallbacks>
{
	std::unordered_map<GameControllerInputSource, Handle<EventCallbackKey>> table;
};

//class GameControllerCallbackKeyTable
//{
//public:
//	using MapType = std::unordered_map<GameControllerInputSource, EventCallbackRegistry::Key>;
//	using Iter = MapType::iterator;
//	using CIter = MapType::const_iterator;
//
//	auto AddKey(GameControllerInputSource source, const EventCallbackRegistry::Key& key) 
//	{ 
//		if (key.eventType != events::GameControllerInput::eventType)
//		{
//			LOG_WARNING("Key's event type must be GameControllerInput");
//
//			return std::make_pair(table_.end(), false);
//		}
//
//		return table_.emplace(source, key); 
//	}
//	auto AddKey(GameControllerInputSource source, EventCallbackRegistry::Key&& key)
//	{
//		if (key.eventType != events::GameControllerInput::eventType)
//		{
//			LOG_WARNING("Key's event type must be GameControllerInput");
//
//			return std::make_pair(table_.end(), false);
//		}
//
//		return table_.emplace(source, std::move(key));
//	}
//
//	EventCallbackRegistry::Key& operator[](GameControllerInputSource source) { return table_[source]; }
//
//	auto Find(GameControllerInputSource source) { return table_.find(source); }
//	auto Find(GameControllerInputSource source) const { return table_.find(source); }
//
//	bool Erase(GameControllerInputSource source) { return table_.erase(source); }
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
//struct GameControllerInputCallbacks : public BaseComponent<GameControllerInputCallbacks>
//{
//	GameControllerCallbackKeyTable table;
//};