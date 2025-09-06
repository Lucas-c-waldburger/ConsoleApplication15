#pragma once
#include "EventCallbackHandle.h"
#include "../core/Dictionary.h"

// Part of its contract is that a callback stored here will never be null
class EventCallbackMasterTable
{
public:
	EventCallbackMasterTable() = default;
	~EventCallbackMasterTable() = default;

	EventCallbackMasterTable(const EventCallbackMasterTable&) = delete;
	EventCallbackMasterTable& operator=(const EventCallbackMasterTable&) = delete;

	EventCallbackMasterTable(EventCallbackMasterTable&& rhs) noexcept :
		table_(std::move(rhs.table_)) {
	}
	EventCallbackMasterTable& operator=(EventCallbackMasterTable&& other) noexcept
	{
		if (this != &other)
		{
			table_ = std::move(other.table_);
		}
		return *this;
	}

	Handle<EventCallback> Insert(EventCallback&& callback)
	{
		const uint32_t eventType = callback.GetEventType();

		if (eventType >= EventDataTypeList::size)
		{
			return {};
		}

		auto& callbackNames = GetUniqueNameMap(eventType);
		if (callbackNames.contains(callback.GetName()))
		{
			return {};
		}

		auto& handleMap = GetHandleMap(eventType);

		auto newHandle = Handle<EventCallback>::Create(eventType);
		assert(newHandle.IsValid());

		auto [it, callbackInserted] = handleMap.try_emplace(newHandle, std::move(callback));
		assert(callbackInserted);

		auto [_, nameInserted] = callbackNames.try_emplace(it->second.GetName(), newHandle);
		assert(nameInserted);

		return newHandle;
	}

	bool Contains(const Handle<EventCallback>& handle) const
	{
		return GetHandleMap(handle.eventType_).contains(handle);
	}
	bool Contains(uint32_t eventType, std::string_view name) const
	{
		return GetUniqueNameMap(eventType).contains(name);
	}

	Result<EventCallback::View> GetCallbackView(const Handle<EventCallback>& handle) const
	{
		if (!handle.IsValid())
		{
			return MAKE_ERROR("Event callback handle was invalid");
		}

		const auto& handleMap = GetHandleMap(handle.eventType_);

		auto it = handleMap.find(handle);
		if (it == handleMap.end())
		{
			return MAKE_ERROR("Event callback for handle not found in registry");
		}

		assert(it->second.IsValid());

		return it->second.MakeView();
	}

	Result<EventCallback::View> GetCallbackView(uint32_t eventType, std::string_view name) const
	{
		if (eventType >= EventDataTypeList::size)
		{
			return MAKE_ERROR("Event type index out of bounds");
		}

		const auto& nameMap = GetUniqueNameMap(eventType);

		auto nameIt = nameMap.find(name);
		if (nameIt == nameMap.end())
		{
			return MAKE_ERROR_FMT("Event callback for name '{}' not found in registry", name);
		}

		return GetCallbackView(nameIt->second);
	}

	Handle<EventCallback> GetCallbackHandle(uint32_t eventType, std::string_view name)
	{
		if (eventType >= EventDataTypeList::size)
		{
			return {};
		}

		const auto& nameMap = GetUniqueNameMap(eventType);

		auto nameIt = nameMap.find(name);

		return (nameIt != nameMap.end()) ? nameIt->second : Handle<EventCallback>{};
	}

	bool Erase(const Handle<EventCallback>& handle)
	{
		if (!handle.IsValid())
		{
			return false;
		}

		auto& handleMap = GetHandleMap(handle.eventType_);

		auto it = handleMap.find(handle);
		if (it == handleMap.end())
		{
			return false;
		}
		
		auto& nameMap = GetUniqueNameMap(handle.eventType_);
		assert(nameMap.contains(it->second.GetName()));

		nameMap.erase(it->second.GetName());
		handleMap.erase(handle);

		return true;
	}

	bool Erase(uint32_t eventType, std::string_view name)
	{
		if (eventType >= EventDataTypeList::size)
		{
			return false;
		}

		auto& nameMap = GetUniqueNameMap(eventType);

		auto it = nameMap.find(name);
		if (it == nameMap.end())
		{
			return false;
		}

		bool erased = Erase(it->second);
		assert(erased);

		return true;
	}

	void Reset()
	{
		for (auto& [handleMap, nameMap] : table_)
		{
			EventCallbackHandleMap{}.swap(handleMap);
			EventCallbackNameMap{}.swap(nameMap);
		}
	}

	void Clear()
	{
		for (auto& [handleMap, nameMap] : table_)
		{
			handleMap.clear();
			nameMap.clear();
		}
	}

private:
	using EventCallbackHandleMap = std::unordered_map<Handle<EventCallback>, EventCallback>;
	using EventCallbackNameMap = UnorderedDictionary<Handle<EventCallback>>;
	using TablePair = std::pair<EventCallbackHandleMap, EventCallbackNameMap>;

	EventCallbackHandleMap& GetHandleMap(uint32_t eventType)
	{
		assert(eventType < EventDataTypeList::size);
		return table_[eventType].first;
	}
	const EventCallbackHandleMap& GetHandleMap(uint32_t eventType) const
	{
		assert(eventType < EventDataTypeList::size);
		return table_[eventType].first;
	}

	EventCallbackNameMap& GetUniqueNameMap(uint32_t eventType)
	{
		assert(eventType < EventDataTypeList::size);
		return table_[eventType].second;
	}
	const EventCallbackNameMap& GetUniqueNameMap(uint32_t eventType) const
	{
		assert(eventType < EventDataTypeList::size);
		return table_[eventType].second;
	}

	std::array<TablePair, EventDataTypeList::size> table_;
};



//struct EventRegistryTest
//{
//public:
//	struct HandleIndex
//	{
//		int index = -1;
//		int slotGen = -1;
//		int registryGen = -1;
//	};
//
//	EventRegistryTest() { ++registryGenCount; }
//
//	using HandleGen = int;
//
//	std::vector<std::pair<EventCallback, HandleGen>> callbacks_;
//	std::vector<int> free_;
//
//	HandleIndex RegisterCallback(EventCallback&& callback)
//	{
//		if (!free_.empty())
//		{
//			int freeIdx = free_.back();
//
//			assert(freeIdx < callbacks_.size());
//
//			auto& [oldCallback, gen] = callbacks_[freeIdx];
//
//			assert(!oldCallback.IsValid());
//
//			oldCallback = std::move(callback);
//
//			free_.pop_back();
//
//			return HandleIndex{
//				.index = freeIdx,
//				.slotGen = gen
//			};
//		}
//
//		callbacks_.emplace_back(std::move(callback), 0);
//
//		return HandleIndex{ 
//			.index = static_cast<int>(callbacks_.size()) - 1, 
//			.slotGen = 0 
//		};
//	}
//
//	bool EraseCallback(HandleIndex handle)
//	{
//		if (handle.index < 0 || handle.index >= callbacks_.size())
//		{
//			return false;
//		}
//
//		auto& [callback, gen] = callbacks_[handle.index];
//		if (handle.slotGen != gen)
//		{
//			return false;
//		}
//
//		callback = EventCallback{};
//		++gen;
//
//		free_.push_back(handle.index);
//
//		return true;
//	}
//
//	EventCallback::ViewSignature GetCallbackView(HandleIndex handle)
//	{
//		if (handle.index < 0 || handle.index >= callbacks_.size())
//		{
//			return nullptr;
//		}
//
//		const auto& [callback, gen] = callbacks_[handle.index];
//		if (handle.slotGen != gen)
//		{
//			return nullptr;
//		}
//
//		return callback.GetFunctionView();
//	}
//
//	size_t GetEffectiveSize() const { return callbacks_.size() - free_.size(); }
//
//	void Reset()
//	{
//		callbacks_.clear();
//		free_.clear();
//	}
//
//	void Compact()
//	{
//		size_t validBack = callbacks_.size();
//		for (int idx : free_)
//		{
//			while (!callbacks_[validBack].first)
//			{
//				--validBack;
//			}
//
//			callbacks_[idx] = std::move(callbacks_[back]);
//		}
//	}
//
//	static inline int registryGenCount = 0;
//};