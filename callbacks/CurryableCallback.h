#pragma once
#include "../core/TypeUtils.h"

template <typename Fn>
class CurryableCallback
{
public:
	template <typename F>
	CurryableCallback(std::string_view nm, F&& fn) :
		name_(std::string{nm}), callback_(std::forward<F>(fn)) {}

	template <typename...Args> requires std::invocable<Fn, Args...>
	auto MakeInstance(Args&&...args)
	{
		return std::invoke(callback_, std::forward<Args>(args)...);
	}

	template <typename Tuple>
	auto MakeInstance(Tuple&& tuple)
	{
		return std::apply(
			[this](auto&&...args) -> decltype(auto) {
				return this->MakeInstance(std::forward<decltype(args)>(args)...);
			},
			std::forward<Tuple>(tuple)
		);
	}

	const std::string& GetName() const { return name_; }

private:
	std::string name_;
	Fn callback_;
};

template <typename F>
CurryableCallback(std::string_view nm, F&& fn) -> CurryableCallback<F>;