#pragma once
#include <array>
#include <vector>
#include <string_view>
#include "../../../atlas/SpriteAtlas.h"

namespace test {

constexpr std::array<std::string_view, 4> kFallAnimSpriteNames = {
	"knight_fall_0", "knight_fall_1", "knight_fall_2", "knight_fall_3"
};

SpriteDescriptorPackage MakeSpriteTestPackage(const std::vector<std::string>& pathStrs,
											  std::string_view seriesName);

} // test