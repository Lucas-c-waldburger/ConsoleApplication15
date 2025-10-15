#pragma once
#include "../events/data/InputEventConcepts.h"
#include "../core/Signal.h"
#include "../events/EventSignalConcepts.h"
#include "../core/Logger.h"

template <SomeInputEvent T>
using InputSignal = Signal<const T&>;

// one for each "Input Event" (e.x. ControllerInputEvent, MouseInputEvent)
template <SomeInputEvent T>
class InputSignalList
{
public:
	using InputEventType = T;
	using InputSourceType = extract_input_event_src_type_t<T>;

	template <typename Fn>
	static constexpr bool ValidFn = 
		ValidInputSignalFnOfType<Fn, InputSourceType, InputEventType>;

	InputSignalList() = default;
	~InputSignalList() = default;

	InputSignalList(const InputSignalList&) = delete;
	InputSignalList& operator=(const InputSignalList&) = delete;

	InputSignalList(InputSignalList&& other) noexcept :
		signals_(std::move(other.signals_)) {
	}
	InputSignalList& operator=(InputSignalList&& other) noexcept
	{
		if (this != &other)
		{
			signals_ = std::move(other.signals_);
		}
		return *this;
	}

	template <typename Fn> requires ValidFn<Fn>
	SignalToken Connect(InputSourceType src, Fn&& fn)
	{
		if (!SizedEnumValueInRange(src))
		{
			LOG_ERROR("Invalid input source value");

			return SignalToken{};
		}

		return signals_[static_cast<size_t>(src)].Connect(std::forward<Fn>(fn));
	}

	// templated source flavor
	template <InputSourceType src, typename Fn> requires ValidFn<Fn>
	SignalToken Connect(Fn&& fn)
	{
		return Connect(src, std::forward<Fn>(fn));
	}

	void Emit(const InputEventType& ev)
	{
		if (!SizedEnumValueInRange(ev.input.source))
		{
			LOG_ERROR("Invalid GameControllerInputSource value");

			return;
		}

		signals_[static_cast<size_t>(ev.input.source)].Emit(ev);
	}


private:
	std::array<InputSignal<InputEventType>, enum_size_v<InputSourceType>> signals_;
};

namespace detail {
template <typename T>
struct is_input_signal_list : std::false_type {};

template <typename T>
struct is_input_signal_list<InputSignalList<T>> : std::true_type {};
} // detail

template <typename T>
concept SomeInputSignalList = detail::is_input_signal_list<T>::value &&
							  SomeInputSourceEnum<typename T::InputSourceType>;