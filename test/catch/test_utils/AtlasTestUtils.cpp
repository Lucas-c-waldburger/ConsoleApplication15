#include "AtlasTestUtils.h"
#include <filesystem>

namespace test {

SpriteDescriptorPackage MakeSpriteTestPackage(const std::vector<std::string>& pathStrs,
											  std::string_view seriesName)
{
	SpriteDescriptorPackage package{};
	package.seriesName = seriesName;
	package.descriptors.resize(pathStrs.size());

	constexpr auto stripName = [](std::string_view pathSv) {
		auto path = std::filesystem::path(pathSv);

		return (std::filesystem::exists(path)) ? path.stem().string() : std::string{};
	};

	std::transform(pathStrs.begin(), pathStrs.end(), package.descriptors.begin(),
		[](auto&& pathStr) {
			return SpriteDescriptor{
				.spriteName = stripName(pathStr),
				.filepath = pathStr
			};
		});

	if (std::any_of(package.descriptors.begin(), package.descriptors.end(),
		[](const auto& desc) { return desc.spriteName.empty() || desc.filepath.empty(); }))
	{
		return {};
	}

	return package;
}

} // test