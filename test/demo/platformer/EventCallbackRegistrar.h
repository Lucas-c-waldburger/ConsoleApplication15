#pragma once
#include "../../../core/Result.h"
#include "../../../core/Dictionary.h"
#include "../../../core/commonObjects.h"
#include "../../../core/FuncTraits.h"
#include "../../../ecs/EntityEvents.h"

class Entity;
class EventBus;

namespace test {

namespace detail {
template <typename Src>
struct event_type_for_src;

template <>
struct event_type_for_src<GameControllerInputSource> {
	using type = events::GameControllerInput;
};

template <>
struct event_type_for_src<MouseInputSource> {
	using type = events::MouseInput;
};

template <>
struct event_type_for_src<KeyboardInputSource> {
	using type = events::KeyboardInput;
};

template <typename Ev>
struct src_for_event_type;

template <>
struct src_for_event_type<events::GameControllerInput> {
	using type = GameControllerInputSource;
};

template <>
struct src_for_event_type<events::MouseInput> {
	using type = MouseInputSource;
};

template <>
struct src_for_event_type<events::KeyboardInput> {
	using type = KeyboardInputSource;
};

template <typename Fn>
struct extract_input_src_type 
{ 
	using type = typename src_for_event_type<
		std::remove_cvref_t<typename FuncTraits<Fn>::arg_at<0>>
	>::type; 
};

} // detail

template <SomeInputSourceEnum Src>
using event_type_for_src_t = typename detail::event_type_for_src<Src>::type;

template <SomeInputEvent Ev>
using src_for_event_type_t = typename detail::src_for_event_type<Ev>::type;

template <typename Fn>
using extract_input_src_type_t = typename detail::extract_input_src_type<Fn>::type;

template <typename Fn>
concept OnEventCompliantFn = requires(EntityEvents& evs, Fn&& fn) {
	{ evs.OnEvent(std::forward<Fn>(fn)) } -> std::same_as<Result<Void>>;
};
template <typename Fn>
concept OnInputCompliantFn = requires(EntityEvents& evs, Fn&& fn) {
	{ evs.OnInput(
		std::declval<extract_input_src_type_t<Fn>>(), 
		std::forward<Fn>(fn)) 
	} -> std::same_as<Result<Void>>;
};

class EventCallbackPrototypeMap
{
public:
	using PrototypeFn = Result<Void>(*)(EntityEvents&&, int64_t);

private:
	struct BracketValueProxy
	{
		friend class EventCallbackPrototypeMap;

		template <OnEventCompliantFn Fn>
		void operator=(Fn&& callback)
		{
			if (!prototypesRef_)
			{
				return;
			}

			(*prototypesRef_)[name_] = [fn = std::move(callback)](EntityEvents&& evs, int64_t)
			-> Result<Void> {
				return evs.OnEvent(std::forward<Fn>(fn));
			});
		}

		template <OnInputCompliantFn Fn>
		void operator=(Fn&& callback)
		{
			if (!prototypesRef_)
			{
				return;
			}

			(*prototypesRef_)[name_] = [fn = std::move(callback)](EntityEvents&& evs, int64_t rawSrc) 
			-> Result<Void> {
				using Src = extract_input_src_type_t<Fn>;

				return evs.OnInput(static_cast<Src>(rawSrc), std::forward<Fn>(fn));
			});
		}

		Result<Void> Register(Entity& e)
		{
			return RegisterImpl(e, 0);
		}

		template <SomeInputSourceEnum Src>
		Result<Void> Register(Entity& e, Src src)
		{
			return RegisterImpl(e, static_cast<int64_t>(src));
		}

	private:
		BracketValueProxy(std::string_view name, UnorderedDictionary<PrototypeFn>& ref) :
			name_(name), prototypesRef_(&ref) {}

		Result<Void> RegisterImpl(Entity& e, int64_t rawSrc)
		{
			if (!busRef_)
			{
				return MAKE_ERROR("EventBus was null");
			}
			if (!prototypesRef_)
			{
				return MAKE_ERROR("Prototype function was null");
			}

			auto it = prototypesRef_->find(name_);
			if (it == prototypesRef_->end())
			{
				return MAKE_ERROR_FMT("Prototype with name '{}' not found", name_);
			}

			return std::invoke(it->second, e.GetEvents(*busRef_), rawSrc);
		}

		std::string_view name_;
		UnorderedDictionary<PrototypeFn>* prototypesRef_ = nullptr;
		EventBus* busRef_ = nullptr;
	};

public:
	BracketValueProxy operator[](std::string_view name)
	{
		return BracketValueProxy{ name, prototypes_ };
	}

	Result<Void> Call(EntityEvents&& evs, std::string_view name, uint64_t rawSrc)
	{
		auto it = prototypes_.find(name);
		if (it == prototypes_.end())
		{
			return MAKE_ERROR_FMT("Prototype with name '{}' already registered", name);
		}

		return std::invoke(it->second, std::move(evs), rawSrc);
	}

	void Clear() { prototypes_.clear(); }

private:
	EventBus* bus_;
	UnorderedDictionary<PrototypeFn> prototypes_;
};

class EventCallbackRegistrar
{
public:
	explicit EventCallbackRegistrar(EventBus& bus, EventCallbackPrototypeMap&& map) :
		bus_(bus), prototypeMap_(std::move(map)) {}

	Result<Void> Register(Entity& e, std::string_view name)
	{
		return prototypeMap_.Call(e.GetEvents(bus_), name, 0);
	}

	template <SomeInputSourceEnum Src>
	void Register(Entity& e, std::string_view name, Src src)
	{
		return prototypeMap_.Call(e.GetEvents(bus_), name, static_cast<int64_t>(src));

	}

private:
	EventBus& bus_;
	EventCallbackPrototypeMap prototypeMap_;
};











} // test