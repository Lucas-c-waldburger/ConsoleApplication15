#pragma once
#include "BaseDriver.h"

//class EventDriver : public BaseDriver<EventDriver, EventCallbacks>
//{
//public:
//	friend class Super;
//
//	template <typename Fn> requires EventCallbackFnCompatible<Fn>
//	Result<Void> SetEventCallback(std::string_view callbackName, Fn&& callbackFn)
//	{
//		auto relations = GetEntity().GetRelations();
//
//		if (relations.IsChild())
//		{
//			return MAKE_ERROR("Entity was a child. children cannot register new callbacks");
//		}
//
//		using EventDataT = std::remove_cvref_t<type_at_index_t<1, typename func_traits<Fn>::arg_types>>;
//
//		relations.AddComponentDelegate(EventCallbackFulfillmentRequest{
//			.callbackName = callbackName,
//			.eventType = EventDataT::eventType,
//			.eventCallback = WrapCallbackFn(std::forward<Fn>(callbackFn))
//		});
//
//		auto& eventCallbacks = GetComponent<EventCallbacks>();
//		eventCallbacks.table[eventType].name = callbackName;
//
//		return Void{};
//	}
//
//	template <SomeEventData T>
//	Result<Void> SetNewEventCallback(std::string_view callbackName, 
//									 TypedLuaFunction<ReturnSignal(Entity&, const T&)> luaFn)
//	{
//		auto relations = GetEntity().GetRelations();
//
//		if (relations.IsChild())
//		{
//			return MAKE_ERROR("Entity was a child. children cannot register new callbacks");
//		}
//
//		relations.AddComponentDelegate(EventCallbackFulfillmentRequest{
//			.callbackName = callbackName,
//			.eventType = T::eventType,
//			.eventCallback = WrapLuaFn(std::move(callbackFn))
//		});
//
//		auto& eventCallbacks = GetComponent<EventCallbacks>();
//		eventCallbacks.table[eventType].name = callbackName;
//
//		return Void{};
//	}
//
//	bool ClearEventCallback(uint32_t eventType)
//	{
//		auto& eventCallbacks = GetComponent<EventCallbacks>();
//		return eventCallbacks.table.erase(eventType);
//	}
//	template <SomeEventData T>
//	bool ClearEventCallback()
//	{
//		auto& eventCallbacks = GetComponent<EventCallbacks>();
//		return eventCallbacks.table.erase(T::eventType);
//	}
//
//private:
//	template <typename Fn> requires EventCallbackFnCompatible<Fn>
//	static EventCallback WrapCallbackFn(Fn&& callbackFn)
//	{
//		using EventDataT = std::remove_cvref_t<type_at_index_t<1, typename func_traits<Fn>::arg_types>>;
//
//		return [fn = std::forward<Fn>(callbackFn)](Entity& entity, const Event& ev) -> ReturnSignal {
//			if (const auto* castEv = EventDataCast<EventDataT>(ev))
//			{
//				return std::invoke(fn, entity, *castEv);
//			}
//
//			return ReturnSignal::KeepObserving;
//		};
//	}
//
//	template <SomeEventData T>
//	static EventCallback WrapLuaFn(TypedLuaFunction<ReturnSignal(Entity&, const T&)> luaFn)
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
//
//	explicit EventDriver(Entity ent) : BaseDriver(ent) {}
//};