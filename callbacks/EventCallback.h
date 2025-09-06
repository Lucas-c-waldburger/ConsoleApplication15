#pragma once
#include "../core/Monitoring.h"
#include "../core/commonObjects.h"
#include "../core/Handle.h"
#include "../core/FuncTraits.h"
#include "../events/Event.h"
#include "../events/EventConcepts.h"
#include "ICallbackSource.h"

class Entity;

//using EventCallback = fu2::unique_function<ReturnSignal(Entity&, const Event&)>;
//using EventCallbackView = fu2::function_view<ReturnSignal(Entity&, const Event&)>;

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
			SomeEventData<std::remove_cvref_t<SecondArg>> && is_const_reference_v<SecondArg>;
	};
} // detail

// CONCEPT
template <typename Fn>
concept EventCallbackFnCompatible = detail::is_event_callback_fn_compatible<Fn>::value;

// TYPE TRAIT HELPER
template <EventCallbackFnCompatible Fn>
using ExtractEventDataTypeFromFnArgs =
	std::remove_cvref_t<type_at_index_t<1, typename func_traits<Fn>::arg_types>>;

// REGISTRATION HELPER ALIAS
template <SomeEventData T>
using EventCallbackSubmission = std::pair<std::string_view,
	fu2::unique_function<ReturnSignal(Entity&, const T&)>>;

//// FORWARD DECL
//template <template <typename, typename...> class FnType, typename NameType>
//class EventCallbackTemplate;
//
//// SPECIALIZATION ALIASES
//using EventCallback = EventCallbackTemplate<fu2::unique_function, std::string>;
//using EventCallbackView = EventCallbackTemplate<fu2::function_view, HashName>;
//
//// DEF
//template <template <typename, typename...> class FnType, typename NameType>
//class EventCallbackTemplate
//{
//public:
//	friend EventCallbackView ToEventCallbackView(EventCallback& eventCallback);
//
//	using Signature = FnType<ReturnSignal(Entity&, const Event&)>;
//
//	template <typename Fn> requires EventCallbackFnCompatible<Fn>
//	explicit EventCallbackTemplate(Fn&& fn) : 
//		eventType_(ExtractEventDataTypeFromFnArgs<Fn>::eventType),
//		fn_(std::forward<Fn>(fn)) {}
//
//	EventCallbackTemplate() = default;
//	~EventCallbackTemplate() = default;
//	EventCallbackTemplate(const EventCallbackTemplate& other) 
//		requires std::copy_constructible<Signature> = default;
//	EventCallbackTemplate& operator=(const EventCallbackTemplate& other)
//		requires std::is_copy_assignable_v<Signature> = default;
//	EventCallbackTemplate(EventCallbackTemplate&& other) noexcept : eventType_(other.eventType_),
//																	fn_(std::move(other.fn_)) {}
//	EventCallbackTemplate& operator=(EventCallbackTemplate&& other) noexcept
//	{
//		if (this != &other)
//		{
//			eventType_ = other.eventType_;
//			fn_ = std::move(other.fn_);
//		}
//		return *this;
//	}
//
//	template <typename...Args> requires std::invocable<Signature, Args...>
//	decltype(auto) operator()(Args&&...args)
//	{
//		assert(fn_);
//		return std::invoke(fn_, std::forward<Args>(args)...);
//	}
//
//	uint32_t GetEventType() const { return eventType_; }
//
//private:
//	
//	uint32_t eventType_ = kInvalidEventType;
//	NameType name_;
//	Signature fn_ = nullptr;
//};
//
//// INSTANTIATIONS
//EventCallbackTemplate<fu2::unique_function, std::string>;
//EventCallbackTemplate<fu2::function_view, HashName>;
//
//// VIEW RETRIEVAL FUNC
//inline EventCallbackView ToEventCallbackView(EventCallback& eventCallback)
//{
//	EventCallbackView fn;
//	fn.eventType_ = eventCallback.eventType_;
//	fn.fn_ = eventCallback.fn_;
//
//	return fn;
//}

//template <template <typename, typename...> class FnType, typename NameType>
//struct EventCallbackData
//{
//	uint32_t eventType_ = kInvalidEventType;
//	NameType name_;
//	Signature fn_ = nullptr;
//};

//template <template <typename, typename...> class FnTemplate, typename FnSig, typename NameType>
//struct BaseCallbackData;
//
//template <template <typename, typename...> class FnTemplate, 
//	      typename Ret, typename...Args, typename NameType>
//struct BaseCallbackData<FnTemplate, Ret(Args...), NameType>
//{
//	using Signature = FnTemplate<Ret(Args...)>;
//
//	NameType name;
//	Signature fn;
//};
//
//template <typename FnSig>
//using ICallbackData = BaseCallbackData<fu2::unique_function, FnSig, std::string>;
//template <typename FnSig>
//using ICallbackViewData = BaseCallbackData<fu2::function_view, FnSig, std::string_view>;
//
//template <template <typename, typename...> class FnTemplate, typename NameType>
//struct BaseEventCallbackData : BaseCallbackData<FnTemplate, ReturnSignal(Entity&, const Event&), NameType>
//{
//	uint32_t eventType = kInvalidEventType;
//};
//
//struct EventCallbackData : BaseEventCallbackData<fu2::unique_function, std::string>
//{ 
//	uint32_t eventType = kInvalidEventType;
//};

//struct EventCallbackData : ICallbackData<ReturnSignal(Entity&, const Event&)>
//{
//	uint32_t eventType = kInvalidEventType;
//};

//template <typename FnSig, int Usage = 0>
//struct ICallbackData
//{
//	std::string name;
//	fu2::unique_function<FnSig> fn;
//};
//
//template <typename FnSig, int Usage = 0>
//struct ICallbackDataView
//{
//	std::string_view name;
//	fu2::function_view<FnSig> fn;
//};



//namespace std {
//	template <typename FnSig, int Usage>
//	struct hash<ICallbackSource<FnSig, Usage>> {
//		size_t operator()(const ICallbackSource<FnSig, Usage>& src) const noexcept {
//			return std::hash<std::string>{}(src.GetName());
//		}
//	};
//} // std

//namespace detail {
//template <typename T, int Usage = 0> 
//struct is_iCallbackSource : std::false_type {};
//template <typename Ret, typename...Args, int Usage>
//struct is_iCallbackSource<Ret(Args...), Usage> : std::true_type {};
//}
//
//template <typename T, int Usage = 0>
//concept SomeICallbackSource = detail::is_iCallbackSource<T>::value;



class EventCallback : public ICallbackSource<ReturnSignal(Entity&, const Event&)>
{
private:
	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	static auto WrapFn(Fn&& callbackFn)
	{
		return [fn = std::forward<Fn>(callbackFn)](Entity& entity, const Event& ev) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<ExtractEventDataTypeFromFnArgs<Fn>>(ev))
			{
				return std::invoke(fn, entity, *castEv);
			}

			return ReturnSignal::KeepObserving;
		};
	}

	template <SomeEventData T>
	static auto WrapLua(TypedLuaFunction<ReturnSignal(Entity&, const T&)> luaFn)
	{
		return [fn = std::move(luaFn)](Entity& entity, const Event& ev) -> ReturnSignal {
			if (const auto* castEv = EventDataCast<T>(ev))
			{
				auto result = fn(entity, *castEv);
				if (!result.Success())
				{
					LOG_ERROR(result.GetError());

					return ReturnSignal::StopObserving;
				}

				return result.GetValue();
			}

			return ReturnSignal::KeepObserving;
		};
	}

	uint32_t eventType_ = kInvalidEventType;

public:
	struct View : ICallbackView<ReturnSignal(Entity&, const Event&)>
	{
		uint32_t eventType = kInvalidEventType;
	};

	EventCallback() = default;

	template <typename Fn> requires EventCallbackFnCompatible<Fn>
	EventCallback(std::string_view name, Fn&& fn) : 
		ICallbackSource(name, WrapFn(std::forward<Fn>(fn))), 
		eventType_(ExtractEventDataTypeFromFnArgs<Fn>::eventType)
	{}	

	template <SomeEventData T>
	EventCallback(std::string_view name, 
		const TypedLuaFunction<ReturnSignal(Entity&, const T&)>& luaFn) :
		ICallbackSource(name, WrapLua(luaFn)),
		eventType_(T::eventType)
	{}

	EventCallback(EventCallback&&) noexcept = default;
	EventCallback& operator=(EventCallback&&) noexcept = default;

	//EventCallback(EventCallback&& other) noexcept : ICallbackSource

	//EventCallback& operator=(EventCallback&& other) noexcept {
	//	if (this != &other)
	//	{
	//		eventType_ = other.eventType_;
	//		ICallbackSource::operator=(std::move(static_cast<ICallbackSource&>(other)));
	//	}
	//	return *this;
	//}

	uint32_t GetEventType() const { return eventType_; }

	View MakeView() const
	{
		View v;
		v.name = GetName();
		v.fn = GetFunctionView();
		v.eventType = eventType_;

		return v;
	}

	bool IsValid() const
	{
		return !GetName().empty() && GetFunctionView() != nullptr && 
			   eventType_ != kInvalidEventType;
	}


};

struct EventCallbackViewTransparentNameHash
{
	using is_transparent = void;

	size_t operator()(std::string_view sv) const noexcept {
		return std::hash<std::string_view>{}(sv);
	}
	size_t operator()(const EventCallback::View& view) const noexcept {
		return std::hash<std::string_view>{}(view.name);
	}
};
struct EventCallbackViewTransparentNameEq
{
	using is_transparent = void;

	bool operator()(std::string_view sv, const EventCallback::View& view) const {
		return sv == view.name;
	}
	bool operator()(const EventCallback::View& view, std::string_view sv) const {
		return sv == view.name;
	}
	bool operator()(const EventCallback::View& lhs, const EventCallback::View& rhs) const {
		return lhs.name == rhs.name;
	}
};

using EventCallbackViewSet = std::unordered_set<EventCallback::View,
	EventCallbackViewTransparentNameHash, EventCallbackViewTransparentNameEq>;





//class EventCallback
//{
//public:
//	using Signature = fu2::unique_function<ReturnSignal(Entity&, const Event&)>;
//	using View = fu2::function_view<ReturnSignal(Entity&, const Event&)>;
//
//	template <typename Fn> requires EventCallbackFnCompatible<Fn>
//	EventCallback(std::string_view name, Fn&& fn) : 
//		eventType_(ExtractEventDataTypeFromFnArgs<Fn>::eventType),
//		name_(std::string{name}),
//		fn_(std::forward<Fn>(fn)) 
//	{}	
//
//	EventCallback() = default;
//	~EventCallback() = default;
//
//	EventCallback(const EventCallback&) = delete;
//	EventCallback& operator=(const EventCallback&) = delete;
//
//	EventCallback(EventCallback&& other) noexcept : eventType_(other.eventType_),
//		name_(std::move(other.name_)), fn_(std::move(other.fn_)) {}
//	EventCallback& operator=(EventCallback&& other) noexcept
//	{
//		if (this != &other)
//		{
//			eventType_ = other.eventType_;
//			name_ = other.name_;
//			fn_ = std::move(other.fn_);
//		}
//		return *this;
//	}
//
//	uint32_t GetEventType() const { return eventType_; }
//	const std::string& GetName() const { return name_; }
//	View MakeView() { return View{ fn_ }; }
//
//private:
//	uint32_t eventType_ = kInvalidEventType;
//	std::string name_;
//	Signature fn_ = nullptr;
//};

