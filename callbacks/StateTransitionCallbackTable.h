//#pragma once
//#include "CallbackTable.h"
//#include "StateTransitionCallback.h"
//#include "../scripting/TypedLuaFunction.h"
//
//template <>
//struct CallbackDetails<StateTransitionCallback> : BaseCallbackDetails {};
//
//class StateTransitionCallbackTable : public CallbackTable<StateTransitionCallback>
//{
//public:
//	template <typename Fn> requires std::convertible_to<Fn, StateTransitionCallback>
//	Token AddCallback(Fn&& fn, DetailsType&& details)
//	{
//		return AddCallbackInternal(std::forward<Fn>(fn), std::move(details));
//	}
//
//	Token AddCallback(const TypedLuaFunction<Void(Entity&)>& luaFn,
//					  DetailsType&& details)
//	{
//		return AddCallbackInternal(WrapLua(luaFn), std::move(details));
//	}
//
//private:
//	static StateTransitionCallback WrapLua(TypedLuaFunction<Void(Entity&)> luaFn)
//	{
//		return [fn = std::move(luaFn)](Entity& entity) -> void {
//			auto result = fn(entity);
//			if (!result.Success())
//			{
//				LOG_ERROR(result.GetError());
//			}
//		};
//	}
//};
