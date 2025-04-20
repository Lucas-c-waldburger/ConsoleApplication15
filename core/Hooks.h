#pragma once
#include <unordered_map>
#include <array>
#include <bitset>
#include <functional>
#include <memory>
#include <cassert>
#include <algorithm>
#include "HandleFactory.h"

class HookPoint
{
public:
	friend class Hooks;
	friend struct HookPointBuilder;

	struct Attachment
	{
		Handle<Attachment> handle;
		std::function<void()> callback;
	};

	enum Identifier : size_t
	{
		SDLEventLoop,
		EventBufferLoop,
		PrePhysicsUpdate,
		PostPhysicsUpdate,
		HOOK_END_SENTINEL
	};

private:
	HookPoint() = default;

	void RunAttached();

	std::vector<Attachment> attachments_;
};

class Hooks
{
public:
	static constexpr size_t kMaxHooks = static_cast<size_t>(HookPoint::HOOK_END_SENTINEL);
	friend class HookPoint;

	~Hooks() = default;

	Hooks(const Hooks&) = delete;
	Hooks(Hooks&&) = delete;
	Hooks& operator=(const Hooks&) = delete;
	Hooks& operator=(Hooks&&) = delete;

	static void SetHookPoint(HookPoint::Identifier ident)
	{
		return Get().SetHookPointImpl(ident);
	}

	template <typename Fn>
	static Handle<HookPoint::Attachment> Attach(HookPoint::Identifier ident, Fn&& fn)
	{
		return Get().AttachImpl(ident, std::forward<Fn>(fn));
	}

	static void Detach(HookPoint::Identifier ident, Handle<HookPoint::Attachment>& handle)
	{
		return Get().DetachImpl(ident, handle);
	}

private:
	struct HookPointBuilder
	{
		HookPoint operator()() const { return HookPoint{}; }
	};

	Hooks();

	void SetHookPointImpl(HookPoint::Identifier ident);

	template <typename Fn>
	Handle<HookPoint::Attachment> AttachImpl(HookPoint::Identifier ident, Fn&& fn);

	void DetachImpl(HookPoint::Identifier ident, Handle<HookPoint::Attachment>& handle);

	static Hooks& Get();

	std::array<HookPoint, kMaxHooks> hookList_;
	HandleFactory<HookPoint::Attachment> handleFactory_;
};

template<typename Fn>
inline Handle<HookPoint::Attachment> Hooks::AttachImpl(HookPoint::Identifier ident, Fn&& fn)
{
	assert(ident < hookList_.size());

	auto newHandle = handleFactory_.GetHandle<HookPoint::Attachment>();

	auto& hookPoint = hookList_[ident];

	hookPoint.attachments_.push_back({ .handle = newHandle, .callback = std::forward<Fn>(fn) });

	return newHandle;
}
