#pragma once
#include "../deps/function2/function2.hpp"
#include <string>
#include <string_view>

template <typename FnSig, int Usage = 0>
class ICallbackSource;

template <typename FnSig, int Usage = 0>
class ICallbackView;

template <typename Ret, typename...Args, int Usage>
class ICallbackSource<Ret(Args...), Usage>
{
public:
	using Signature = fu2::unique_function<Ret(Args...)>;
	using ViewSignature = fu2::function_view<Ret(Args...)>;

	ICallbackSource(std::string_view name, Signature&& fn) : 
		name_(std::string{ name }), fn_(std::move(fn)) {}

	template <typename Fn> requires std::convertible_to<Fn, Signature>
	ICallbackSource(std::string_view name, Fn&& fn) :
		name_(std::string{ name }), fn_(std::forward<Fn>(fn)) 
	{}

	ICallbackSource() = default;
	~ICallbackSource() = default;

	ICallbackSource(const ICallbackSource&) = delete;
	ICallbackSource& operator=(const ICallbackSource&) = delete;

	ICallbackSource(ICallbackSource&& other) noexcept :
		name_(std::move(other.name_)), fn_(std::move(other.fn_)) {
	}
	ICallbackSource& operator=(ICallbackSource&& other) noexcept {
		if (this != &other)
		{
			name_ = std::move(other.name_);
			fn_ = std::move(other.fn_);
		}
		return *this;
	}

	const std::string& GetName() const { return name_; }
	ViewSignature GetFunctionView() const { return ViewSignature{ fn_ }; }

	bool operator==(const ICallbackSource& rhs) const
	{
		return name_ == rhs.name_;
	}

	bool operator!() const { return !fn_; }

private:
	std::string name_;
	Signature fn_ = nullptr;
};


template <typename Ret, typename...Args, int Usage>
struct ICallbackView<Ret(Args...), Usage>
{
	std::string_view name;
	fu2::function_view<Ret(Args...)> fn;
};