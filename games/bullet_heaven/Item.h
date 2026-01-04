#pragma once
#include <cstdint>
#include <limits>

namespace game {

using ItemID = size_t;

inline constexpr ItemID kInvalidItemID = std::numeric_limits<ItemID>::max();

} // game