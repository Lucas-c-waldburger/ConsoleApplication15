#pragma once
#include <functional>
#include <type_traits>
#include "../deps/function2/function2.hpp"
#include "TypeUtils.h"

namespace detail {

template <typename T, typename = void>
struct func_traits_impl {};

template <typename Ret, typename... Args>
struct func_traits_impl<Ret(*)(Args...), void>
{
	using return_type = Ret;
	using arg_types = TypeList<Args...>;
	static constexpr std::size_t argCount = sizeof...(Args);

	template <size_t I> requires (I < argCount)
		using arg_at = type_at_index_t<I, arg_types>;
};

// simple function
template <typename Ret, typename...Args>
struct func_traits_impl<Ret(Args...), void> : func_traits_impl<Ret(*)(Args...)> {};

// member function
template <typename Class, typename Ret, typename... Args>
struct func_traits_impl<Ret(Class::*)(Args...), void> : func_traits_impl<Ret(*)(Args...)> {};

template <typename Class, typename Ret, typename... Args>
struct func_traits_impl<Ret(Class::*)(Args...) const, void> : func_traits_impl<Ret(*)(Args...)> {};

// std::function
template <typename Ret, typename... Args>
struct func_traits_impl<std::function<Ret(Args...)>, void> : func_traits_impl<Ret(*)(Args...)> {};

// fu2::unique_function
template <typename Ret, typename... Args>
struct func_traits_impl<fu2::unique_function<Ret(Args...)>, void> : func_traits_impl<Ret(*)(Args...)> {};

// fu2::function_view
template <typename Ret, typename... Args>
struct func_traits_impl<fu2::function_view<Ret(Args...)>, void> : func_traits_impl<Ret(*)(Args...)> {};

// lambda 
template <typename T>
struct func_traits_impl<T, std::void_t<decltype(&T::operator())>>
	: func_traits_impl<decltype(&T::operator())> {
};

} // detail

template <typename T>
struct func_traits : detail::func_traits_impl<std::remove_cv_t<T>> {};

// CONCEPT REQUIRE
template <typename T>
concept HasFuncTraits = requires() {
	typename func_traits<T>::return_type;
	typename func_traits<T>::arg_types;
};