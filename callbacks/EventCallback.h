#pragma once
#include "../core/Monitoring.h"
#include "../core/commonObjects.h"
#include "../core/FuncTraits.h"
#include "../events/Event.h"
#include "../events/EventConcepts.h"

class Entity;

using EventCallback = fu2::unique_function<ReturnSignal(Entity&, const Event&)>;
struct EventCallbackView
{
	HashName name;
	fu2::function_view<ReturnSignal(Entity&, const Event&)> fn;
};

namespace detail {
	template <typename Fn>
	struct is_event_callback_fn_compatible
	{
		static_assert(HasFuncTraits<Fn>);
		static_assert(func_traits<Fn>::arg_types::size == 2);

		using Ret = typename func_traits<Fn>::return_type;
		using FirstArg = type_at_index_t<0, typename func_traits<Fn>::arg_types>;
		using SecondArg = type_at_index_t<1, typename func_traits<Fn>::arg_types>;

		static constexpr bool value =
			std::same_as<Ret, ReturnSignal> &&
			std::same_as<FirstArg, Entity&> &&
			SomeEventData<std::remove_cvref_t<SecondArg>>&& is_const_reference_v<SecondArg>;
	};
} // detail

template <typename Fn>
concept EventCallbackFnCompatible = detail::is_event_callback_fn_compatible<Fn>::value;