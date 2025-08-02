//#pragma once
//#include "Callback.h"
//#include "../deps/function2/function2.hpp"
//#include <unordered_map>
//
//template <SomeCallbackTypedefs T>
//class CallbackTable
//{
//public:
//	class Token
//	{
//	public:
//		friend class CallbackTable;
//
//		Token() = default;
//		~Token() { if (onDestroy_) { onDestroy_(*this); } }
//
//		Token(const Token&) = delete;
//		Token& operator=(const Token&) = delete;
//
//		Token(Token&& other) noexcept : onDestroy_(std::move(other.onDestroy_))
//		{
//			other.onDestroy_ = nullptr;
//		}
//
//		Token& operator=(Token&& other) noexcept
//		{
//			if (this != &other) 
//			{
//				if (onDestroy_) onDestroy_();
//				onDestroy_ = std::move(other.onDestroy_);
//				other.onDestroy_ = nullptr;
//			}
//			return *this;
//		}
//
//		Token Clone() const
//		{
//			// if onDestroy null, the callback is shareable and can be cloned 
//			return (!onDestroy_) ? Token{ id_, nullptr } : Token{};
//		}
//
//	private:
//		Token(int id, fu2::function_view<void(const Token&)> cleanup) :
//			id_(id), onDestroy_(cleanup) {}
//
//		int id_ = -1;
//		fu2::function_view<void(const Token&)> onDestroy_;
//	};
//
//	CallbackTable() : tokenCleanupFn_([this](const Token& tk) { 
//		if (TokenValid(tk)) { Erase(tk); } 
//	}) {}
//
//	typename T::View GetView(const Token& token)
//	{
//		auto it = callbackSlots_.find(token.id_);
//
//		return (it != callbackSlots_.end()) ? it->second.callback : nullptr;
//	}
//
//	bool Erase(const Token& token)
//	{
//		return callbackSlots_.erase(token.id_);
//	}
//
//	const typename T::Details* GetDetails(const Token& token) const
//	{
//		auto it = callbackSlots_.find(token.id_);
//
//		return (it != callbackSlots_.end()) ? it->second.details : nullptr;
//	}
//
//	bool TokenValid(const Token& token) const
//	{
//		callbackSlots_.contains(token.id_);
//	}
//
//private:
//	struct CallbackSlot
//	{
//		typename T::Function callback;
//		typename T::Details details;
//	};
//
//	Token MakeToken(CallbackLifetime lifetime)
//	{
//		static int tokenCount = 0;
//
//		return (lifetime == CallbackLifetime::Scoped) ?
//			Token{ tokenCount++, tokenCleanupFn_ } :
//			Token{ tokenCount++, nullptr };	
//	}
//
//	std::unordered_map<int, CallbackSlot> callbackSlots_;
//	fu2::unique_function<void(const Token&)> tokenCleanupFn_;
//
//protected:
//	Token AddCallbackInternal(typename T::Function&& cb, typename T::Details&& details)
//	{
//		CallbackLifetime lifetime = details.lifetime;
//
//		callbackSlots_.emplace(std::move(cb), std::move(details));
//
//		return MakeToken(lifetime);
//	}
//};