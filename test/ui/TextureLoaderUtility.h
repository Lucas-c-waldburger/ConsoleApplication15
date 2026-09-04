#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../deps/tinyfiledialogs/tinyfiledialogs.h"
#include "../../atlas/NewTextureRepository.h"
#include "../../file/FilePathUtility.h"
#include <ranges>

namespace ui {

class TextureLoaderUtility
{
public:
	static Result<bool> HandleSpriteSelection(SDL_Renderer* r, TextureRepository& repo)
	{
		static constexpr const char* filters[] = {
			".png",
			".jpg",
			".jpeg",
			".bmp",
			".tga",
			".gif",
			".webp",
			".svg"
		};

		const char* selections = tinyfd_openFileDialog(
			"Load Sprite",
			GetDefaultSpriteFolderString().c_str(),
			1,
			filters,
			".png",
			1
		);

		if (!selections)
		{
			return false;
		}

		SpriteDescriptors descriptors{
			.data = ParseMultiplePathSelect(selections) | std::views::transform([](auto&& str) {
				return SpriteDescriptor{
					.filepath = std::string{str}
				};
			}) | std::ranges::to<std::vector>()
		};

		TRY(repo.GetSpriteAtlas().LoadSprites(r, std::move(descriptors)));

		return true;
	}

	static Result<bool> HandleFontSelection(SDL_Renderer* r, TextureRepository& repo, int fontSize)
	{
		static constexpr const char* filters[] = { ".ttf" };

		const char* selections = tinyfd_openFileDialog(
			"Load Font",
			GetDefaultFontFolderString().c_str(),
			1,
			filters,
			".ttf",
			1
		);
		 
		if (!selections)
		{
			return false;
		}

		auto descriptors = ParseMultiplePathSelect(selections) | 
			std::views::transform([size = std::max(fontSize, 1)](auto&& str) {
				return FontDescriptor{
					.filepath = std::string{str},
					.fontSize = size
				};
			}) | std::ranges::to<std::vector>();

		TRY(repo.GetFontAtlas().LoadFonts(r, std::move(descriptors)));

		return true;
	}

	static Result<bool> HandleFontFolderSelection(SDL_Renderer* r, TextureRepository& repo, 
												  int fontSize)
	{
		const char* selectedFolder = tinyfd_selectFolderDialog(
			"Load Font Directory",
			GetDefaultFontFolderString().c_str()
		);

		if (!selectedFolder)
		{
			return false;
		}

		FontDescriptors descriptors;
		int size = std::max(1, fontSize);
		for (auto& entry : fs::directory_iterator(selectedFolder))
		{
			descriptors.push_back({ .filepath = entry.path().string(), .fontSize = size });
		}

		TRY(repo.GetFontAtlas().LoadFonts(r, std::move(descriptors)));

		return true;
	}

private:
	static const std::string& GetDefaultSpriteFolderString()
	{
		static const std::filesystem::path kDefaultSpriteFolderPath =
			FilePathUtility::GetRootPath() / "resources" / "sprites" / "";

		static const std::string kDefaultSpriteFolderString =
			kDefaultSpriteFolderPath.string();

		return kDefaultSpriteFolderString;
	}

	static const std::string& GetDefaultFontFolderString()
	{
		static const std::filesystem::path kDefaultFontFolderPath =
			FilePathUtility::GetRootPath() / "resources" / "fonts" / "";

		static const std::string kDefaultFontFolderString =
			kDefaultFontFolderPath.string();

		return kDefaultFontFolderString;
	}

	static std::vector<std::string_view> ParseMultiplePathSelect(const char* selection)
	{
		std::vector<std::string_view> paths;

		if (!selection)
		{
			return paths;
		}

		std::string_view selectionView{ selection };

		while (!selectionView.empty())
		{
			const auto pos = selectionView.find('|');

			const auto path = selectionView.substr(0, pos);
			paths.emplace_back(path);

			if (pos == std::string_view::npos)
			{
				break;
			}

			selectionView.remove_prefix(pos + 1);
		}

		return paths;
	}
};



} // ui

#endif