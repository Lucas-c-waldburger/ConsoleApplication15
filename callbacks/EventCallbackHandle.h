//#pragma once
//#include "../core/Handle.h"
//#include "EventCallback.h"
//
//template <>
//class Handle<EventCallback> : public IHandle<Handle<EventCallback>>
//{
//public:
//	friend class Super;
//	friend class EventCallbackMasterTable;
//
//	Handle() = default;
//
//	bool operator==(const Handle& rhs) const
//	{
//		return id_ == rhs.id_ && gen_ == rhs.gen_ &&
//			   eventType_ == rhs.eventType_;
//	}
//
//	uint32_t GetEventType() const { return eventType_; }
//
//private:
//	bool IsValidImpl() const
//	{
//		return (id_ > -1 && id_ < idCount) &&
//			   (gen_ > -1 && gen_ <= genCount) &&
//			   (eventType_ < EventDataTypeList::size);
//	}
//
//	size_t GetHashImpl() const noexcept
//	{
//		size_t hash = 0;
//		HashCombine(hash, std::hash<int>{}(id_));
//		HashCombine(hash, std::hash<int>{}(gen_));
//		HashCombine(hash, std::hash<uint32_t>{}(eventType_));
//
//		return hash;
//	}
//
//	static Handle CreateImpl(uint32_t eventType)
//	{
//		return Handle{ idCount++, genCount, eventType };
//	}
//
//	static inline int idCount = 0;
//	static inline int genCount = 0;
//
//	Handle(int id, int gen, uint32_t eventType) :
//		id_(id), gen_(gen), eventType_(eventType) {}
//
//	int id_ = -1;
//	int gen_ = -1;
//	uint32_t eventType_ = kInvalidEventType;
//};