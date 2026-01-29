#pragma once
#include <functional>
#include "../deps/function2/function2.hpp"
#include "TypeUtils.h"
#include "../scripting/TypedLuaFunction.h"
// TRY TO REMOVE LUA INCLUDE AND FWD DECLARE

// lambda / functor
template <typename T>
struct func_traits : func_traits<decltype(&T::operator())> {};

//template <typename ClassType, typename Ret, typename...Args>
//struct func_traits<Ret(ClassType::*)(Args...) const>
//{
//    using return_type = Ret;
//    using arg_types = std::tuple<Args...>;
//    static constexpr std::size_t argCount = sizeof...(Args);
//
//    template <size_t I> requires (I < argCount)
//    using arg_at = std::tuple_element_t<I, arg_types>;
//};
//
//// Non-const lambda specialization 
//template <typename ClassType, typename Ret, typename...Args>
//struct func_traits<Ret(ClassType::*)(Args...)> {
//    using return_type = Ret;
//    using arg_types = std::tuple<Args...>;
//    static constexpr std::size_t argCount = sizeof...(Args);
//
//    template <size_t I> requires (I < argCount)
//    using arg_at = std::tuple_element_t<I, arg_types>;
//};

// function pointer
template <typename Ret, typename... Args>
struct func_traits<Ret(*)(Args...)>
{
	using return_type = Ret;
	using arg_types = TypeList<Args...>;
    static constexpr std::size_t argCount = sizeof...(Args);

    template <size_t I> requires (I < argCount)
    using arg_at = type_at_index_t<I, arg_types>;
};

// member function pointer (non-const)
template <typename Class, typename Ret, typename... Args>
struct func_traits<Ret(Class::*)(Args...)> : func_traits<Ret(*)(Args...)> {};

// member function pointer (const)
template <typename Class, typename Ret, typename... Args>
struct func_traits<Ret(Class::*)(Args...) const> : func_traits<Ret(*)(Args...)> {};

// std::function
template <typename Ret, typename... Args>
struct func_traits<std::function<Ret(Args...)>> : func_traits<Ret(*)(Args...)> {};

// fu2::unique_function
template <typename Ret, typename... Args>
struct func_traits<fu2::unique_function<Ret(Args...)>> : func_traits<Ret(*)(Args...)> {};

// fu2::function_view
template <typename Ret, typename... Args>
struct func_traits<fu2::function_view<Ret(Args...)>> : func_traits<Ret(*)(Args...)> {};

// TypedLuaFunction
//template <typename Sig>
//struct TypedLuaFunction; // fwd decl

template <typename Ret, typename... Args>
struct func_traits<TypedLuaFunction<Ret(Args...)>> : func_traits<Ret(*)(Args...)> {};


// CONCEPT REQUIRE
template <typename T>
concept HasFuncTraits = requires() {
	typename func_traits<T>::return_type;
	typename func_traits<T>::arg_types;
};

//// LAMBDA TRAITS
//template <typename T>
//struct lambda_traits : lambda_traits<decltype(&T::operator())> {};
//
//// Const lambda specialization
//template <typename ClassType, typename Ret, typename...Args>
//struct lambda_traits<Ret(ClassType::*)(Args...) const> 
//{
//    using return_type = Ret;
//    using arg_types = std::tuple<Args...>;
//    static constexpr std::size_t arity = sizeof...(Args);
//};
//
//// Non-const lambda specialization 
//template <typename ClassType, typename Ret, typename...Args>
//struct lambda_traits<Ret(ClassType::*)(Args...)> {
//    using return_type = Ret;
//    using arg_types = std::tuple<Args...>;
//    static constexpr std::size_t arity = sizeof...(Args);
//};