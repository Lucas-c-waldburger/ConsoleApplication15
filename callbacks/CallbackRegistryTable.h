#pragma once
//#include <cassert>
//#include <vector>
//#include <unordered_map>
//#include <algorithm>
//#include "../deps/function2/function2.hpp"
//#include "BaseCallbackDescriptor.h"
//#include "../core/Handle.h"
//#include "../core/Monitoring.h"
//
//class Entity;
//
//template <SomeCallbackDescriptor Descriptor, typename...CallbackArgs>
//class CallbackRegistryTable
//{
//public:
//	using CallbackFn = fu2::unique_function<ReturnSignal(Entity&, CallbackArgs...)>;
//	using CallbackFnView = fu2::function_view<ReturnSignal(Entity&, CallbackArgs...)>;
//
//	CallbackRegistryTable() = default;
//	~CallbackRegistryTable() = default;
//
//	CallbackRegistryTable(const CallbackRegistryTable&) = delete;
//	CallbackRegistryTable& operator=(const CallbackRegistryTable&) = delete;
//
//	CallbackRegistryTable(CallbackRegistryTable&& other) noexcept : 
//		callbacks_(std::move(other.callbacks_)), descriptors_(std::move(other.descriptors_)) {}
//	CallbackRegistryTable& operator=(CallbackRegistryTable&& other) noexcept
//	{
//		if (this != &other)
//		{
//			callbacks_ = std::move(other.callbacks_);
//			descriptors_ = std::move(other.descriptors_);
//		}
//		return *this;
//	}
//
//	Handle<Descriptor> Insert(const Descriptor& desc, CallbackFn&& fn)
//	{		
//		if (DescriptorRegistered(desc))
//		{
//			LOG_ERROR("Duplicate callback descriptor in callback registry");
//
//			return {};
//		}
//
//		auto handle = Handle<Descriptor>::Create();
//		size_t newIdx = descriptors_.size();
//
//		descriptors_.emplace_back(std::make_pair(desc, handle));
//		callbacks_.emplace(std::make_pair(handle, std::make_pair(std::move(fn), newIdx)));
//
//		return handle;
//	}
//
//	Handle<Descriptor> Insert(Descriptor&& desc, CallbackFn&& fn)
//	{
//		if (DescriptorRegistered(desc))
//		{
//			LOG_ERROR("Duplicate callback descriptor in callback registry");
//
//			return {};
//		}
//
//		auto handle = Handle<Descriptor>::Create();
//		size_t newIdx = descriptors_.size();
//
//		descriptors_.emplace_back(std::make_pair(std::move(desc), handle));
//		callbacks_.emplace(std::make_pair(handle, std::make_pair(std::move(fn), newIdx)));
//
//		return handle;
//	}
//
//	bool Erase(const Handle<Descriptor>& handle)
//	{
//		auto it = callbacks_.find(handle);
//		if (it == callbacks_.end())
//		{
//			return false;
//		}
//
//		size_t keyIdx = it->second.second;
//		size_t backIdx = descriptors_.size() - 1;
//		assert(keyIdx <= backIdx);
//
//		if (keyIdx != backIdx)
//		{
//			auto backHandle = descriptors_[backIdx].second;
//
//			std::swap(descriptors_[keyIdx], descriptors_[backIdx]);
//
//			assert(callbacks_.contains(backHandle));
//			callbacks_[backHandle].second = keyIdx;
//		}
//
//		descriptors_.pop_back();
//
//		return true;
//	}
//
//	size_t EraseAllWithOwner(Entity_t owner)
//	{
//		assert(owner != kInvalidEntity); 
//		
//		size_t eraseCount = 0;
//		for (size_t i = 0; i < descriptors_.size();)
//		{
//			const auto& [desc, handle] = descriptors_[i];
//
//			if (desc.uniqueOwner.value_or(kInvalidEntity) == owner)
//			{
//				Erase(handle);
//
//				++eraseCount;
//			}
//			else
//			{
//				++i;
//			}
//		}
//
//		return eraseCount;
//	}
//
//	bool Contains(const Descriptor& desc) const
//	{
//		auto it = std::find_if(descriptors_.begin(), descriptors_.end(),
//			[&desc](const auto& pair) { return pair.first == desc; });
//		if (it != descriptors_.end())
//		{
//			assert(callbacks_.contains(it->second));
//
//			return true;
//		}
//
//		return false;
//	}
//
//	Handle<Descriptor> GetHandle(const Descriptor& desc) const
//	{
//		auto it = std::find_if(descriptors_.begin(), descriptors_.end(),
//			[&desc](const auto& pair) { return pair.first == desc; });
//		if (it == descriptors_.end())
//		{
//			LOG_ERROR("Callback descriptor not mapped to a handle");
//
//			return {};
//		}
//
//		return it->second;
//	}
//
//	CallbackFnView GetCallbackView(const Handle<Descriptor>& handle)
//	{
//		auto it = callbacks_.find(handle);
//		if (it == callbacks_.end())
//		{
//			LOG_ERROR("Handle not mapped to a callback");
//
//			return nullptr;
//		}
//
//		return CallbackFnView{ it->second.first };
//	}
//
//	const Descriptor* GetCallbackDescriptor(const Handle<Descriptor>& handle) const
//	{
//		auto it = callbacks_.find(handle);
//		if (it == callbacks_.end())
//		{
//			LOG_ERROR("Handle not mapped to a callback descriptor");
//
//			return nullptr;
//		}
//
//		size_t descriptorIdx = it->second.second;
//		assert(descriptorIdx < descriptors_.size());
//
//		return &descriptors_[descriptorIdx].first;
//	}
//
//private:
//	bool DescriptorRegistered (const Descriptor& desc) const
//	{
//		return std::find_if(descriptors_.begin(), descriptors_.end(), 
//			[&desc](const auto& pair) { return pair.first == desc; }) != descriptors_.end();
//	}
//
//	std::vector<std::pair<Descriptor, Handle<Descriptor>>> descriptors_;
//	std::unordered_map<Handle<Descriptor>, std::pair<CallbackFn, size_t>> callbacks_;
//};