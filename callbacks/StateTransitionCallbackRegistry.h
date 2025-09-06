#pragma once
//#include "StateTransitionCallback.h"
//#include "../core/commonObjects.h"
//#include "../scripting/TypedLuaFunction.h"

//class StateTransitionCallbackRegistry
//{
//public:
//	StateTransitionCallbackRegistry() = default;
//	~StateTransitionCallbackRegistry() = default;
//
//	StateTransitionCallbackRegistry(const StateTransitionCallbackRegistry&) = delete;
//	StateTransitionCallbackRegistry& operator=(const StateTransitionCallbackRegistry&) = delete;
//
//	StateTransitionCallbackRegistry(StateTransitionCallbackRegistry&& rhs) noexcept :
//		masterTable_(std::move(rhs.masterTable_)) {
//	}
//	StateTransitionCallbackRegistry& operator=(StateTransitionCallbackRegistry&& other) noexcept
//	{
//		if (this != &other)
//		{
//			masterTable_ = std::move(other.masterTable_);
//		}
//		return *this;
//	}
//
//	template <typename Fn> requires std::convertible_to<Fn, typename StateTransitionCallback::Signature>
//	StateTransitionCallback::View RegisterCallback(std::string_view callbackName, Fn&& callbackFn)
//	{
//		if (callbackName.empty() || masterTable_.contains(callbackName))
//		{
//			return {};
//		}
//
//		auto [it, inserted] = masterTable_.emplace(
//			StateTransitionCallback{ callbackName, std::forward<Fn>(callbackFn) });
//		assert(inserted);
//
//		return it->MakeView();
//	}
//
//	StateTransitionCallback::View RegisterCallback(std::string_view callbackName,
//												   const TypedLuaFunction<Void(Entity&)>& luaFn)
//	{
//		if (callbackName.empty() || masterTable_.contains(callbackName))
//		{
//			return {};
//		}
//
//		auto [it, inserted] = masterTable_.emplace(
//			StateTransitionCallback{ callbackName, luaFn });
//		assert(inserted);
//
//		it->MakeView();
//	}
//
//	bool EraseCallback(std::string_view callbackName)
//	{
//		auto it = masterTable_.find(callbackName);
//		if (it == masterTable_.end())
//		{
//			return false;
//		}
//
//		masterTable_.erase(it);
//
//		return true;
//	}
//
//	StateTransitionCallback::View GetCallbackView(std::string_view callbackName) const
//	{
//		auto it = masterTable_.find(callbackName);
//		if (it == masterTable_.end())
//		{
//			return {};
//		}
//
//		return it->MakeView();
//	}
//
//	bool HasValidCallback(std::string_view callbackName) const
//	{
//		auto it = masterTable_.find(callbackName);
//
//		return it != masterTable_.end() && it->IsValid();
//	}
//
//private:
//	struct StateTransitionCallbackTransparentHash
//	{
//		using is_transparent = void;
//
//		size_t operator()(std::string_view sv) const noexcept {
//			return std::hash<std::string_view>{}(sv);
//		}
//		size_t operator()(const StateTransitionCallback& stcb) const noexcept {
//			return std::hash<std::string_view>{}(stcb.GetName());
//		}
//	};
//
//	struct StateTransitionCallbackTransparentEq
//	{
//		using is_transparent = void;
//
//		bool operator()(std::string_view sv, const StateTransitionCallback& stcb) const {
//			return sv == stcb.GetName();
//		}
//		bool operator()(const StateTransitionCallback& lhs, const StateTransitionCallback& rhs) const {
//			return lhs.GetName() == rhs.GetName();
//		}
//	};
//
//	using StateTransitionCallbackSet = std::unordered_set<StateTransitionCallback, 
//		StateTransitionCallbackTransparentHash, StateTransitionCallbackTransparentEq>;
//
//	StateTransitionCallbackSet masterTable_;
//
//	//HashNameMap<StateTransitionCallback> masterTable_;
//};