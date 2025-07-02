#pragma once
#include <cassert>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "../deps/function2/function2.hpp"
#include "BaseCallbackKey.h"
#include "../core/Handle.h"
#include "../core/Monitoring.h"

template <typename DerivedKey, typename...CallbackArgs>
class CallbackRegistryTable
{
public:
	struct CallbackDetails
	{
		DerivedKey key;
		Handle<DerivedKey> handle;
	};

	using CallbackFn = fu2::unique_function<ReturnSignal(Entity_t, CallbackArgs...)>;
	using CallbackFnView = fu2::function_view<ReturnSignal(Entity_t, CallbackArgs...)>;

	CallbackRegistryTable() = default;
	~CallbackRegistryTable() = default;

	CallbackRegistryTable(const CallbackRegistryTable&) = delete;
	CallbackRegistryTable& operator=(const CallbackRegistryTable&) = delete;

	CallbackRegistryTable(CallbackRegistryTable&& other) noexcept : 
		callbacks_(std::move(other.callbacks_)), keys_(std::move(other.keys_)) {}
	CallbackRegistryTable& operator=(CallbackRegistryTable&& other) noexcept
	{
		if (this != &other)
		{
			callbacks_ = std::move(other.callbacks_);
			keys_ = std::move(other.keys_);
		}
		return *this;
	}

	Handle<DerivedKey> Insert(const DerivedKey& key, CallbackFn&& fn)
	{
		if (keys_.contains(key))
		{
			LOG_ERROR("Duplicate key in callback registry");

			return {};
		}

		auto handle = Handle<DerivedKey>::Create();

		keys_.emplace(std::make_pair(key, handle));
		callbacks_.emplace(std::make_pair(handle, std::move(fn)));

		return handle;
	}

	Handle<DerivedKey> Insert(DerivedKey&& key, CallbackFn&& fn)
	{
		if (keys_.contains(key))
		{
			LOG_ERROR("Duplicate key in callback registry");

			return {};
		}

		auto handle = Handle<DerivedKey>::Create();

		keys_.emplace(std::make_pair(std::move(key), handle));
		callbacks_.emplace(std::make_pair(handle, std::move(fn)));

		return handle;
	}

	bool Erase(const DerivedKey& key)
	{
		auto it = keys_.find(key);
		if (it == keys_.end())
		{
			return false;
		}

		auto handle = it->second;

		keys_.erase(it);

		bool callbackErased = callbacks_.erase(handle);
		assert(callbackErased);

		return true;
	}

	bool Erase(const Handle<DerivedKey>& handle)
	{
		bool erased = std::erase_if(keys_, [handle](const auto& pair) { 
			return pair.second == handle; 
		});

		if (!erased)
		{
			return false;
		}

		bool callbackErased = callbacks_.erase(handle);
		assert(callbackErased);

		return true;
	}

	size_t EraseAllWithOwner(Entity_t owner)
	{
		assert(owner != kInvalidEntity);
		size_t eraseCount = 0;

		for (auto it = keys_.begin(); it != keys_.end();)
		{
			const auto& [key, handle] = *it;

			if (key.uniqueOwner.value_or(kInvalidEntity) == owner)
			{
				bool callbackErased = callbacks_.erase(handle);
				assert(callbackErased);

				it = keys_.erase(it);

				++eraseCount;
			}
			else
			{
				++it;
			}
		}

		return eraseCount;
	}

	bool Contains(const DerivedKey& key) const
	{
		auto it = keys_.find(key);
		if (it != keys_.end())
		{
			const auto& handle = it->second;
			assert(callbacks_.contains(handle));

			return true;
		}

		return false;
	}

	Handle<DerivedKey> GetHandle(const DerivedKey& key) const
	{
		auto it = keys_.find(key);
		if (it == keys_.end())
		{
			LOG_ERROR("Key not mapped to a handle");

			return {};
		}

		assert(it->second.IsValid());

		return it->second;
	}

	CallbackFnView GetCallbackView(const Handle<DerivedKey>& handle) const
	{
		auto it = callbacks_.find(handle);
		if (it == callbacks_.end())
		{
			LOG_ERROR("Handle not mapped to a callback");

			return nullptr;
		}

		return CallbackFnView{ it->second };
	}

	std::vector<CallbackDetails> FindCallbackDetailsByName(std::string_view name) const
	{
		std::vector<CallbackDetails> details;

		for (const auto& [key, handle] : keys_)
		{
			if (key.callbackName == name)
			{
				details.emplace_back(key, handle);
			}
		}

		return details;
	}

	std::vector<CallbackDetails> FindCallbackDetailsByOwner(Entity_t owner) const
	{
		std::vector<CallbackDetails> details;

		for (const auto& [key, handle] : keys_)
		{
			if (key.uniqueOwner.has_value() && *key.uniqueOwner == owner)
			{
				details.emplace_back(key, handle);
			}
		}

		return details;
	}

private:
	std::unordered_map<Handle<DerivedKey>, CallbackFn> callbacks_;
	std::unordered_map<DerivedKey, Handle<DerivedKey>> keys_;
};