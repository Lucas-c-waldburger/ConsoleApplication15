#pragma once
#include "ICallbackSource.h"
#include "../scripting/TypedLuaFunction.h"
#include "../core/Result.h"
#include "../systems/SystemRegistry.h"

//using EventCallback2 = fu2::unique_function<ReturnSignal(Entity&, const Event&)>;
//
//class EventCoordinator : EventDispatchListener<EventCoordinator>
//{
//public:
//		
//	void Dispatch(EventSpan events);
//	
//
//private:
//	std::array<EventCallback2, EventDataTypeList::size> onDemandListeners_;
//	EventCallbackSystem eventCallbackSystem_;
//};






//class Entity;
//
//class StateTransitionCallback : public ICallbackSource<void(Entity&)>
//{
//private:
//	static auto WrapLua(TypedLuaFunction<Void(Entity&)> luaFn);
//
//public:
//	struct View : ICallbackView<void(Entity&)> {};
//
//	StateTransitionCallback() = default;
//
//	template <typename Fn> requires std::convertible_to<Fn, Signature>
//	StateTransitionCallback(std::string_view name, Fn&& fn) : 
//		ICallbackSource(name, std::forward<Fn>(fn)) 
//	{}
//
//	StateTransitionCallback(std::string_view name,
//		const TypedLuaFunction<Void(Entity&)>& luaFn);
//
//	View MakeView() const
//	{
//		View v;
//		v.name = GetName();
//		v.fn = GetFunctionView();
//
//		return v;
//	}
//
//	bool IsValid() const
//	{
//		return !GetName().empty() && GetFunctionView() != nullptr;
//	}
//};

//using StateTransitionCallback = fu2::unique_function<void(Entity&)>;
//using StateTransitionCallbackView = fu2::function_view<void(Entity&)>;
//
//struct StateTransitionCallbackKey
//{
//	HashName name = kInvalidHashName;
//	StateTransitionCallbackView fn;
//};