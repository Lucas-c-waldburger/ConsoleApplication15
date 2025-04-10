#pragma once
#include <unordered_map>
#include <array>
#include <bitset>
#include <functional>
#include <memory>
#include <cassert>
#include "HandleFactory.h"

class HookPoint
{
private:
	friend class Hooks;

	struct Attachment
	{
		Handle<Attachment> handle;
		std::function<void()> callback;
	};

	void RunAttached();

	std::vector<Attachment> attachments_;

public:
	enum Identifier : size_t
	{
		SDLEventLoop,
		EventBufferLoop,
		PrePhysicsUpdate,
		PostPhysicsUpdate,
		ALL,
		HOOK_END_SENTINEL
	};

	template <typename Fn>
	Handle<Attachment> Attach(Fn&& fn);

	void Detach(Handle<Attachment>& handle);
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
		return Get().SetImpl(ident);
	}

	static std::unique_ptr<HookPoint>& GetHookPoint(HookPoint::Identifier ident)
	{
		return Get().GetHookPointImpl(ident);
	}

private:
	Hooks() = default;

	void SetImpl(HookPoint::Identifier ident);

	std::unique_ptr<HookPoint>& GetHookPointImpl(HookPoint::Identifier ident);

	static Hooks& Get();

	std::array<std::unique_ptr<HookPoint>, kMaxHooks> hookList_;
	HandleFactory<HookPoint::Attachment> handleFactory_;
};

template<typename Fn>
inline Handle<HookPoint::Attachment> HookPoint::Attach(Fn&& fn)
{
	auto newHandle = Hooks::Get().handleFactory_.GetHandle<Attachment>();

	attachments_.push_back({ .handle = newHandle, .callback = std::forward<Fn>(fn) });

	return newHandle;
}



//class Hook;
//
//using HookCallback = std::function<void()>;
//
//class HookManager
//{
//public:
//	friend class Hook;
//	static constexpr size_t kMaxHooks = static_cast<size_t>(HookPoint::HOOK_END_SENTINEL);
//
//	~HookManager() = default;
//
//	HookManager(const HookManager&) = delete;
//	HookManager(HookManager&&) = delete;
//	HookManager& operator=(const HookManager&) = delete;
//	HookManager& operator=(HookManager&&) = delete;
//
//	template <typename...Ts> requires (std::same_as<HookPoint, Ts> && ...)
//	static void EnableHooks(Ts...hps)
//	{
//		return HookManager::Get().EnableHooksImpl(std::forward<Ts>(hps)...);
//	}
//
//	template <typename...Ts> requires (std::same_as<HookPoint, Ts> && ...)
//	static void DisableHooks(Ts...hps)
//	{
//		return HookManager::Get().DisableHooksImpl(std::forward<Ts>(hps)...);
//	}
//
//	template <HookPoint hp, typename Fn> requires (static_cast<size_t>(hp) < kMaxHooks)
//	static void Attach(std::string name, Fn&& attachable)
//	{
//		return HookManager::Get().AttachImpl<hp>(std::move(name), std::forward<Fn>(attachable));
//	}
//
//	template <typename Fn>
//	static void Attach(HookPoint hp, std::string name, Fn&& attachable)
//	{
//		assert(static_cast<size_t>(hp) < kMaxHooks);
//		return HookManager::Get().AttachImpl(hp, std::move(name), std::forward<Fn>(attachable));
//	}
//
//	template <HookPoint hp> requires (static_cast<size_t>(hp) < kMaxHooks)
//	static void Detach(std::string_view name)
//	{
//		return HookManager::Get().DetachImpl<hp>(name);
//	}
//
//	static void Detach(HookPoint hp, std::string_view name)
//	{
//		assert(static_cast<size_t>(hp) < kMaxHooks);
//		return HookManager::Get().DetachImpl(hp, name);
//	}
//
//private:
//	HookManager() = default;
//
//	enum HookFlag : uint8_t
//	{
//		Active = 1 << 0,
//		Registered = 1 << 1
//	};
//
//	static HookManager& Get()
//	{
//		static std::unique_ptr<HookManager> instance;
//		if (!instance)
//		{
//			instance = std::unique_ptr<HookManager>(new HookManager{});
//		}
//
//		return *instance;
//	}
//
//	template <HookPoint hp>
//	void RegisterHook(std::shared_ptr<Hook> hook);
//
//	template <typename T> requires std::same_as<HookPoint, T>
//	static constexpr bool IsAllHooksImpl(T hp) { return hp == HookPoint::ALL; }
//
//	template <typename...Ts> requires (std::same_as<HookPoint, Ts> && ...)
//	static constexpr bool IsAllHooks(Ts...hps) { return ((IsAllHooksImpl(hps)) || ...); }
//
//	void EnableAllImpl() { activeStates_.set(); }
//	void DisableAllImpl() { activeStates_.reset(); };
//
//	template <typename...Ts> requires (std::same_as<HookPoint, Ts> && ...)
//	void EnableHooksImpl(Ts...hps)
//	{
//		assert(((static_cast<size_t>(hps) < kMaxHooks) && ...));
//		if (IsAllHooks(hps...))
//		{
//			EnableAllImpl();
//			return;
//		}
//
//		((activeStates_.set(static_cast<size_t>(hps), true)), ...);
//	}
//
//	template <typename...Ts> requires (std::same_as<HookPoint, Ts> && ...)
//	void DisableHooksImpl(Ts...hps)
//	{
//		assert(((static_cast<size_t>(hps) < kMaxHooks) && ...));
//		if (IsAllHooks(hps...))
//		{
//			DisableAllImpl();
//			return;
//		}
//
//		((activeStates_.set(static_cast<size_t>(hps), false)), ...);
//	}
//
//
//	template <HookPoint hp, typename Fn>
//	void AttachImpl(std::string&& name, Fn&& attachable);
//
//	template <typename Fn>
//	void AttachImpl(HookPoint hp, std::string&& name, Fn&& attachable);
//
//	template <HookPoint hp>
//	void DetachImpl(std::string_view name);
//
//	void DetachImpl(HookPoint hp, std::string_view name);
//
//	std::array<std::shared_ptr<Hook>, kMaxHooks> hookList_;
//	std::bitset<kMaxHooks> activeStates_{};
//};
//
//
//class Hook
//{
//public:
//	friend class HookManager;
//
//	~Hook() = default;
//
//	Hook(const Hook&) = delete;
//	Hook(Hook&&) = delete;
//	Hook& operator=(const Hook&) = delete;
//	Hook& operator=(Hook&&) = delete;
//
//	template <HookPoint hp>
//	static void Set();
//
//private:
//	Hook() = default;
//
//	void RunAttached();
//
//	std::unordered_map<std::string, HookCallback> attached_;
//	HookManager::HookFlag flags = HookManager::Active;
//};
//
//
//template<HookPoint hp>
//inline void HookManager::RegisterHook(std::shared_ptr<Hook> hook)
//{
//	assert((hookList_[static_cast<size_t>(hp)]->flags & HookFlag::Registered) == 0);
//
//	hookList_[static_cast<size_t>(hp)] = std::move(hook);
//}
//
//template<HookPoint hp, typename Fn>
//inline void HookManager::AttachImpl(std::string&& name, Fn&& attachable)
//{
//	return AttachImpl(hp, std::move(name), std::forward<Fn>(attachable));
//}
//
//template<typename Fn>
//inline void HookManager::AttachImpl(HookPoint hp, std::string&& name, Fn&& attachable)
//{
//	auto& hook = hookList_[static_cast<size_t>(hp)];
//
//	assert(hook);
//	assert(hook->flags & HookFlag::Registered);
//
//	if (hook->attached_.contains(name))
//	{
//		LOG_WARNING_FMT("Overwriting hook attachment with name '{}'", name);
//	}
//
//	hook->attached_[std::move(name)] = std::forward<Fn>(attachable);
//}
//
//template<HookPoint hp>
//inline void HookManager::DetachImpl(std::string_view name)
//{
//	return DetachImpl(hp, name);
//}
//
//template<HookPoint hp>
//inline void Hook::Set()
//{
//	static std::shared_ptr<Hook> instance;
//	if (!instance)
//	{
//		instance = std::shared_ptr<Hook>(new Hook{});
//	}
//
//	HookManager::Get().RegisterHook<hp>(instance);
//}


