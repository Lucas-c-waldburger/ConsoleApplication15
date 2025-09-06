#pragma once
#include <string_view>

namespace test {

constexpr std::string_view kWalkSeriesName = "walk";
constexpr std::string_view kJumpSeriesName = "jump";
constexpr std::string_view kFallSeriesName = "fall";
constexpr std::string_view kIdleSeriesName = "idle";
constexpr std::string_view kLookUpSeriesName = "idle";

constexpr std::string_view kWalkStateName = "walk_state";
constexpr std::string_view kJumpStateName = "jump_state";
constexpr std::string_view kFallStateName = "fall_state";

}