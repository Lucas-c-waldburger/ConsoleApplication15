//#pragma once
//#include "CallbackTable.h"
//#include "EventCallback.h"
//#include "../events/EventUtils.h"
//
//
//
//class EventCallbackTable : public CallbackTable<EventCallback>
//{
//public:
//	template <typename Fn> requires EventCallbackFnCompatible<Fn>
//	Token AddCallback(Fn&& fn, EventCallback::Details&& details)
//	{
//		details.eventType = EventDataArg<Fn>::eventType;
//
//		return AddCallbackInternal(WrapFn(std::forward<Fn>(fn)), std::move(details));
//	}
//
//	template <SomeEventData T>
//	Token AddCallback(const TypedLuaFunction<ReturnSignal(Entity&, const T&)>& luaFn,
//					  EventCallback::Details&& details)
//	{
//		details.eventType = T::eventType;
//
//		return AddCallbackInternal(WrapLua(luaFn), std::move(details));
//	}
//
//private:
//	template <typename Fn>
//	using EventDataArg = std::remove_cvref_t<type_at_index_t<1,
//		typename func_traits<Fn>::arg_types>>;
//
//	template <typename Fn>
//	static EventCallback::Function WrapFn(Fn&& callbackFn)
//	{
//		return [fn = std::forward<Fn>(callbackFn)](Entity& entity, const Event& ev) -> ReturnSignal {
//			if (const auto* castEv = EventDataCast<EventDataArg<Fn>>(ev))
//			{
//				return std::invoke(fn, entity, *castEv);
//			}
//
//			return ReturnSignal::KeepObserving;
//		};
//	}
//
//	template <SomeEventData T>
//	static EventCallback::Function WrapLua(TypedLuaFunction<ReturnSignal(Entity&, const T&)> luaFn)
//	{
//		return [fn = std::move(luaFn)](Entity& entity, const Event& ev) -> ReturnSignal {
//			if (const auto* castEv = EventDataCast<T>(ev))
//			{
//				auto result = fn(entity, *castEv);
//				if (!result.Success())
//				{
//					LOG_ERROR(result.GetError());
//
//					return ReturnSignal::StopObserving;
//				}
//
//				return result.GetValue();
//			}
//
//			return ReturnSignal::KeepObserving;
//		};
//	}
//};
//
//
