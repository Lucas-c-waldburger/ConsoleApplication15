//#pragma once
//#include "CallbackTable.h"
//
//template <typename T>
//class CallbackRegistry
//{
//public:
//	template <typename Fn>
//	typename T::Token RegisterShared(const std::string& name, Fn&& fn) // <- return Result<Token>
//	{
//		if (referenceDictionary_.contains(name))
//		{
//			return typename T::Token{};
//		}
//
//		typename T::DetailsType details{};
//		details.name = name;
//		details.lifetime = CallbackLifetime::Persisted;
//
//		auto token = callbackTable_.AddCallback(std::forward<Fn>(fn), std::move(details));
//
//		referenceDictionary_[name] = token.Clone();
//
//		return token;
//	}
//
//	template <typename Fn>
//	typename T::Token RegisterUnique(Fn&& fn) // <- return Result<Token>
//	{
//		typename T::DetailsType details{};
//		details.lifetime = CallbackLifetime::Scoped;
//
//		return callbackTable_.AddCallback(std::forward<Fn>(fn), std::move(details));
//	}
//
//	typename T::Token GetCallbackToken(const std::string& name) const
//	{
//		auto it = referenceDictionary_.find(name);
//
//		return (it != referenceDictionary_.end()) ? it->second.Clone() : typename T::Token{};
//	}
//
//	const typename T::DetailsType* GetCallbackDetails(const std::string& name) const
//	{
//		auto it = referenceDictionary_.find(name);
//
//		return (it != referenceDictionary_.end()) ? callbackTable_.GetDetails(it->second) : nullptr;
//	}
//
//	const typename T::DetailsType* GetCallbackDetails(const typename T::Token& token) const
//	{
//		return callbackTable_.GetDetails(token);
//	}
//
//	decltype(auto) GetCallbackView(const typename T::Token& token)
//	{
//		return callbackTable_.GetCallback(token);
//	}
//
//	bool TokenValid(const typename T::Token& token) const
//	{
//		return callbackTable_.TokenValid(token);
//	}
//
//	bool EraseCallback(const typename T::Token& token)
//	{
//		if (!callbackTable_.TokenValid(token))
//		{
//			return false;
//		}
//
//		const auto* details = callbackTable_.GetDetails(token);
//		if (!details->callbackName.empty())
//		{
//			bool erased = callbackTable_.erase(details->callbackName);
//			assert(erased);
//		}
//
//		return callbackTable_.Erase(token);
//	}
//
//	const auto& GetDictionary() { return referenceDictionary_; }
//	const auto& GetCallbackTable() { return callbackTable_; }
//
//private:
//	T callbackTable_;
//	std::unordered_map<std::string, typename T::Token> referenceDictionary_;
//};