#pragma once
#include <array>
#include "../core/Signal.h"
#include "../core/SizedEnum.h"
#include "../inputs/controller/GameControllerInputSource.h"

using ControllerInputSignal = Signal<const events::GameControllerInput&>;

class ControllerInputSignalList
{
public:
	using Source = GameControllerInputSource;

	ControllerInputSignalList() = default;
	~ControllerInputSignalList() = default;

	ControllerInputSignalList(const ControllerInputSignalList&) = delete;
	ControllerInputSignalList& operator=(const ControllerInputSignalList&) = delete;

	ControllerInputSignalList(ControllerInputSignalList&& other) noexcept :
		signals_(std::move(other.signals_)) {
	}
	ControllerInputSignalList& operator=(ControllerInputSignalList&& other) noexcept
	{
		if (this != &other)
		{
			signals_ = std::move(other.signals_);
		}
		return *this;
	}

	template <typename Fn>
		requires std::convertible_to<Fn, typename ControllerInputSignal::SlotCallbackType>
	SignalToken Connect(Source src, Fn&& fn)
	{
		if (!SizedEnumValueInRange(src))
		{
			LOG_ERROR("Invalid GameControllerInputSource value");

			return SignalToken{};
		}

		return signals_[static_cast<size_t>(src)].Connect(std::forward<Fn>(fn));
	}

	// templated source flavor
	template <Source src, typename Fn>
		requires std::convertible_to<Fn, typename ControllerInputSignal::SlotCallbackType>
	SignalToken Connect(Fn&& fn)
	{
		return Connect(src, std::forward<Fn>(fn));
	}

	void Emit(const events::GameControllerInput& ev)
	{
		if (!SizedEnumValueInRange(ev.input.source))
		{
			LOG_ERROR("Invalid GameControllerInputSource value");

			return;
		}

		signals_[static_cast<size_t>(ev.input.source)].Emit(ev);
	}


private:
	std::array<ControllerInputSignal, enum_size_v<GameControllerInputSource>> signals_;
};