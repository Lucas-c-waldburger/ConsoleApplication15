#pragma once
#include <unordered_map>
#include <array>
#include <bitset>
#include <functional>
#include <memory>
#include <cassert>
#include <algorithm>
#include "HandleFactory.h"
#include "Algorithms.h"
#include "Monitoring.h"

enum class HookPoint
{
	LoopStart,
	LoopEnd,
	MAX_INDEX
};

template <typename T>
concept SomeEnumClass = std::is_enum_v<T> && !std::is_convertible_v<T, std::underlying_type_t<T>>;

template <typename T>
concept SomeIndexingEnum = 
	SomeEnumClass<T> && // enum class
	std::same_as<decltype(T::MAX_INDEX), T> &&  // has a value named MAX_INDEX
	static_cast<std::underlying_type_t<T>>(T::MAX_INDEX) > 0; // at least make sure its not 0 or neg


template <typename T, SomeIndexingEnum E, int IdxOffset = 0>
class EnumIndexedArray
{
public:
	constexpr T& operator[](E idx)
	{
		size_t actualIdx = static_cast<size_t>(idx) + IdxOffset;
		assert(actualIdx < N);
		return array_[actualIdx];
	}
	constexpr const T& operator[](E idx) const
	{
		size_t actualIdx = static_cast<size_t>(idx) + IdxOffset;
		assert(actualIdx < N);
		return array_[actualIdx];
	}

private:
	static constexpr size_t N = static_cast<size_t>(E::MAX_INDEX);
	std::array<T, N> array_;
};

struct HookAttachment;

template <>
class Handle<HookAttachment>
{
public:
	template <typename...HandleTs>
	friend class HandleFactory;

	Handle() = default;
	bool operator==(const Handle&) const = default;
	size_t GetHash() const noexcept
	{
		return static_cast<size_t>(id_) * 31 + static_cast<size_t>(gen_);
	}

	bool IsValid() const
	{
		using ValueType = std::underlying_type_t<HookPoint>;

		return id_ <= idCount && gen_ == genCount && 
			   static_cast<ValueType>(hookPoint_) >= 0 &&
			   static_cast<ValueType>(hookPoint_) < static_cast<ValueType>(HookPoint::MAX_INDEX);
	}

	friend std::ostream& operator<<(std::ostream& os, const Handle<HookAttachment>& handle)
	{
		if (!handle.IsValid())
		{
			os << "{ INVALID }";
		}
		else
		{
			os << "{ id: " << handle.id_ << ", gen: " << handle.gen_ << " }";
		}

		return os;
	}

	static Handle Create(HookPoint hp)
	{
		return Handle{ idCount++, genCount, hp };
	}

	HookPoint GetHookPoint() const
	{
		return hookPoint_;
	}

private:
	static inline int idCount = 0;
	static inline int genCount = 0;

	Handle(int id, int gen, HookPoint hp) : id_(id), gen_(gen), hookPoint_(hp) {}

	int id_ = -1;
	int gen_ = -1;

	HookPoint hookPoint_ = HookPoint::MAX_INDEX;
};

struct HookAttachment
{
	std::function<ReturnSignal()> callback;
	ReturnSignal status;
	Handle<HookAttachment> handle;
};


class HookManager
{
public:
	template <HookPoint hp>
	void SetHookPoint() 
	{
		auto it = hooks_[hp].begin();
		while (it != hooks_[hp].end())
		{
			auto& [callback, status, handle] = *it;

			assert(handle.IsValid());
			assert(status != ReturnSignal::Unknown); 
			assert(status != ReturnSignal::StopObserving); // should have been removed from last run

			if (!callback)
			{
				it = hooks_[hp].erase(it);
				continue;
			}

			if (status == ReturnSignal::Pause)
			{
				++it;
				continue;
			}

			status = callback();
			if (status == ReturnSignal::StopObserving)
			{
				it = hooks_[hp].erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	template <typename Fn>
	Handle<HookAttachment> Attach(HookPoint hp, Fn&& fn, ReturnSignal initialStatus = ReturnSignal::KeepObserving)
	{
		auto newHandle = Handle<HookAttachment>::Create(hp);
		assert(newHandle.IsValid());

		HookAttachment attachment{
			.callback = std::forward<Fn>(fn),
			.status = initialStatus,
			.handle = newHandle
		};

		hooks_[hp].push_back(std::move(attachment));

		return newHandle;
	}

	bool Detach(const Handle<HookAttachment>& handle)
	{
		if (!handle.IsValid())
		{
			return false;
		}

		const HookPoint hp = handle.GetHookPoint();

		return EraseIf(hooks_[hp], [&handle](const auto& attachment) { 
			return attachment.handle == handle; 
		});
	}

	bool SetAttachmentStatus(const Handle<HookAttachment>& handle, ReturnSignal newStatus)
	{
		if (!handle.IsValid())
		{
			return false;
		}

		const HookPoint hp = handle.GetHookPoint();

		auto it = FindIf(hooks_[hp], [&handle](const auto& attachment) {
			return attachment.handle == handle;
		});

		if (it == hooks_[hp].end())
		{
			return false;
		}

		it->status = newStatus;

		return true;
	}

private:
	using HookAttachmentArray = EnumIndexedArray<std::vector<HookAttachment>, HookPoint>;
	HookAttachmentArray hooks_;
};

