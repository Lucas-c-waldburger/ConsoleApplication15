#pragma once
#include <concepts>

template <typename Derived>
class Pausable
{
public:
	void SetPaused(bool doPause)
	{
		if (doPause == paused_)
		{
			return;
		}

		if constexpr (requires (Derived& d) {
			{ d.SetPausedImpl(true) } -> std::same_as<void>;
		})
		{
			static_cast<Derived*>(this)->SetPausedImpl(doPause);
		}

		paused_ = doPause;
	}

	bool IsPaused() const noexcept { return paused_; }

private:
	bool paused_ = false;
};

template <typename T>
concept SomePausable = std::derived_from<T, Pausable<T>>;