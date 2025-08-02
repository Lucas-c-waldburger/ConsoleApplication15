//#pragma once
//#include "BaseCallbackDetails.h"
//#include "../deps/function2/function2.hpp"
//
//template <typename T>
//struct CallbackDetails;
//
//template <typename T>
//class Callback;
//
//// just a typedef holder
//template <typename Ret, typename...Args>
//class Callback<Ret(Args...)>
//{
//public:
//	using Function = fu2::unique_function<Ret(Args...)>;
//	using View     = fu2::function_view<Ret(Args...)>;
//	using Details  = CallbackDetails<Ret(Args...)>;
//	static_assert(std::derived_from<Details, BaseCallbackDetails>);
//
//private:
//	Callback() = default;
//};
//
//
//template <typename T>
//concept SomeCallbackTypedefs = requires() {
//	typename T::Function;
//	typename T::View;
//	typename T::Details;
//};
//
//
////template <typename T>
////class CallbackView;
//
////template <typename Ret, typename...Args>
////class TestRegistry
////{
////public:
////
////private:
////	static inline int callbackId = 0;
////
////	std::unordered_map<int, fu2::unique_function<Ret(Args...)>> callbacks_;
////};
////
////template <typename Ret, typename...Args>
////struct CallbackControlBlock
////{
////	fu2::function_view<Ret(Args...)> view;
////	int gen = 0;
////};
////
////template <typename Ret, typename...Args>
////class CallbackViewProvider;
////
////template <typename Ret, typename...Args>
////class CallbackView
////{
////public:
////	friend class CallbackViewProvider<Ret, Args...>;
////
////	CallbackView() = default;
////
////	operator bool()() const { return Valid(); }
////	bool operator!() const { return !Valid(); }
////
////	template <typename...Ts> requires std::invocable<fu2::function_view<Ret(Args...)>, Ts...>
////	Ret operator()(Ts&&...args)
////	{
////		assert(Valid());
////		return std::invoke(controlBlock_.view, std::forward<Ts>(args)...);
////	}
////
////private:
////	CallbackView(CallbackControlBlock<Ret, Args...>& controlBlock) :
////		controlBlock_(&controlBlock), gen_(controlBlock.gen) {}
////
////	bool Valid() const
////	{
////		return controlBlock_ && controlBlock_.view && controlBlock_.gen == gen_;
////	}
////
////	CallbackControlBlock<Ret, Args...>* controlBlock_;
////	int gen_ = -1;
////};
////
////template <typename Ret, typename...Args>
////class CallbackViewProvider
////{
////public:
////	template <typename Fn> requires std::convertible_to<Fn, fu2::unique_function<Ret, Args...>>
////	void SetCallback(Fn&& fn)
////	{
////		function_ = std::forward<Fn>(fn);
////		controlBlock_.view = function_;
////		++controlBlock_.gen;
////	}
////
////	void Clear()
////	{
////		function_ = nullptr;
////		controlBlock_.view = nullptr;
////		++controlBlock_.gen;
////	}
////
////	CallbackView<Ret, Args...> GetView()
////	{
////		return CallbackView<Ret, Args...>{ controlBlock_ };
////	}
////
////	bool Available() const { return function_ == nullptr; }
////
////private:
////	fu2::unique_function<Ret, Args...> function_;
////	CallbackControlBlock<Ret, Args...> controlBlock_;
////};
////
////template <size_t N>
//
//
//
////template <typename Ret, typename...Args>
////class CallbackControlBlocks
////{
////public:
////
////
////private:
////	std::vector<CallbackControlBlock<Ret, Args...>> blocks_;
////	size_t head_ = 0;
////};
//
//
////template <typename Ret, typename...Args>
////class CallbackViewProducer;
////
////template <typename Ret, typename...Args>
////class CallbackView
////{
////public:
////	friend class CallbackViewProducer<Ret, Args...>;
////
////	CallbackView() = default;
////
////	operator bool()() const { return view_ != nullptr; }
////	bool operator!() const { return view_ == nullptr; }
////	
////	template <typename...Ts> requires std::invocable<fu2::function_view<Ret(Args...)>, Ts...>
////	Ret operator()(Ts&&...args)
////	{
////		assert(view_);
////		return std::invoke(view_, std::forward<Ts>(args)...);
////	}
////
////private:
////	CallbackView(fu2::function_view<Ret(Args)..> view, int id) : view_(view), id_(id) {}
////
////	fu2::function_view<Ret(Args...)> view_;
////	int parentId_ = -1;
////};
////
////template <typename Ret, typename...Args>
////class CallbackViewProducer
////{
////public:
////	CallbackViewProducer() = default;
////
////	template <typename Fn> requires std::convertible_to<Fn, fu2::unique_function<Ret, Args...>>
////	explicit CallbackViewProducer(Fn&& fn)
////	{
////
////	}
////
////
////	CallbackView<Ret, Args...> GetView()
////	{
////
////	}
////
////private:
////	auto GetCleanupLambda(CallbackView<Ret, Args...>& view)
////	{
////		return [&view]
////	}
////
////	fu2::unique_function<Ret(Args...)> function_;
////	
////	int id_ = -1;
////};