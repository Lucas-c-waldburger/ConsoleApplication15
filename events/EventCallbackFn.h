#pragma once
#include "../deps/function2/function2.hpp"
#include "EventConcepts.h"
#include "../core/FuncTraits.h"
#include "../core/Monitoring.h"

//using EventCallbackFn = fu2::unique_function<ReturnSignal(const Event&, Entity_t)>;
//using EventCallbackView = fu2::function_view<ReturnSignal(const Event&, Entity_t)>;

//namespace detail {
//template <typename Fn>
//struct is_event_callback_fn_compatible
//{
//	static_assert(HasFuncTraits<Fn>);
//	static_assert(func_traits<Fn>::arg_types::size == 2);
//
//	using Ret = typename func_traits<Fn>::return_type;
//	using FirstArg = type_at_index_t<0, typename func_traits<Fn>::arg_types>;
//	using SecondArg = type_at_index_t<1, typename func_traits<Fn>::arg_types>;
//
//	static constexpr bool value =
//		std::same_as<Ret, ReturnSignal>&&
//		SomeEventData<std::remove_cvref_t<FirstArg>>&& is_const_reference_v<FirstArg>&&
//		std::same_as<SecondArg, Entity_t>;
//};
//} // detail
//
//template <typename Fn>
//concept EventCallbackFnCompatible = detail::is_event_callback_fn_compatible<Fn>::value;