#pragma once
#include <ranges>
#include "../../../../core/WeightGenerator.h";
#include "../../../../sdl/SDLUtils.h"

namespace test {

struct CooldownTimer
{
	float duration = 0.0f;
	float elapsed = 0.0f;

	void Update(float dt) noexcept { if (Running() && !Ready()) { elapsed += dt; } }
	bool Ready() const noexcept { return elapsed >= duration; }
	bool Running() const noexcept { return elapsed >= 0.0f; }

	void Pause() noexcept { elapsed = -1.0f; }
	void Reset() noexcept { elapsed = 0.0f; }
};

template <typename T, bool grow> requires (std::is_floating_point_v<T> || 
										   std::same_as<T, SDL_FPoint>)
class SpreadValue
{
public:
	class TimeSpread
	{
	public:
		friend class SpreadValue;

		TimeSpread() = default;

		void Update(float dt)
		{
			assert(ValidState());

			accum_ += dt;

			if (accum_ > std::abs(stepDurations_[currentIdx_]) &&
				currentIdx_ < stepDurations_.size() - 1)
			{
				++currentIdx_;
				accum_ = 0.0f;
			}
		}

		// since time durations must be positive values, flip the sign to 
		// flag the value at the current index as already retrieved
		T RetrieveCurrentValue() const
		{
			assert(ValidState());

			if (!ReadyToRetrieve())
			{
				return T{};
			}

			stepDurations_[currentIdx_] = -stepDurations_[currentIdx_];

			return values_[currentIdx_];
		}

		bool ReadyToRetrieve() const
		{
			assert(ValidState());

			return stepDurations_[currentIdx_] > 0.0f;
		}

		bool Done() const noexcept
		{
			assert(ValidState());

			return currentIdx_ == stepDurations_.size() - 1 &&
				   accum_ > stepDurations_[currentIdx_] &&
				   stepDurations_[currentIdx_] < 0.0f;
		}

	private:
		bool ValidState() const noexcept
		{
			const bool equalSize = stepDurations_.size() == values_.size();
			const bool notEmpty = !stepDurations_.empty();
			const bool idxInRange = currentIdx_ < stepDurations_.size();

			return equalSize && notEmpty && idxInRange;
		}

		std::vector<T> values_;
		std::vector<float> stepDurations_;
		float accum_ = 0.0f;
		size_t currentIdx_ = 0;
	};

	class FrameSpread
	{
	public:
		friend class SpreadValue;

		FrameSpread() = default;

		void Update() { ++frameCount_; }

		T GetCurrentValue() const
		{
			assert(frameCount_ > 0 && static_cast<size_t>(frameCount_) < values_.size());
			return values_[static_cast<size_t>(frameCount_)];
		}

		bool Done() const noexcept
		{
			values_.empty() || frameCount_ < 0 || 
				static_cast<size_t>(frameCount_) >= values_.size();
		}

	private:
		std::vector<T> values_;
		int frameCount_ = -1;
	};

	explicit constexpr SpreadValue(T totalVal) : value_(totalVal) {}

	TimeSpread OverTime(float sec, size_t steps, double falloff) const
	{
		TimeSpread spread{};

		spread.values_ = WeightGenerator::GenerateDecayWeights(
			static_cast<int>(steps), falloff, grow) |
			std::views::transform([val = this->value_](const double weight) {
				return val * weight;
			}) | std::ranges::to<std::vector>();

		spread.stepDurations_ = WeightGenerator::GenerateDecayWeights(
			static_cast<int>(steps), falloff, grow) |
			std::views::transform([sec](const double weight) {
				return sec * weight;	
			}) | std::ranges::to<std::vector>();

		return spread;
	}

	FrameSpread OverFrames(size_t frames, double falloff) const
	{
		FrameSpread spread{ .frameCount = frames };

		spread.values = WeightGenerator::GenerateGaussianWeights(
			static_cast<int>(frames), falloff, grow) |
			std::views::transform([val = this->value_](const double weight) {
				return val * weight; 
			}) | std::ranges::to<std::vector>();

		return spread;
	}

private:
	T value_;
};

template <typename T> using GrowValue = SpreadValue<T, true>;
template <typename T> using DecayValue = SpreadValue<T, false>;


} // test