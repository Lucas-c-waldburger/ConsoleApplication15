#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <string_view>
#include <filesystem>
#include "../../core/commonObjects.h"
#include "../../core/Result.h"

class SceneFixture;

namespace ui {

//class FileDialog
//{
//public:
//	enum class IconType
//	{
//		None,
//		Info,
//		Error,
//		Warning,
//		Question
//	};
//
//	static void NotifyPopup(std::string_view title, std::string_view msg, IconType iconType);
//
//
//
//private:
//	FileDialog() = default;
//};

class SceneSaveUtility
{
public:
	struct SceneOpenResponse
	{
		bool success = false;
		std::vector<Error> errors;
	};

	static Result<Void> HandleSceneSave(const SceneFixture& fixture);
	static Result<Void> HandleSceneSaveAs(const SceneFixture& fixture);

	static std::optional<std::filesystem::path> QuerySceneOpen();
	static SceneOpenResponse HandleSceneOpen(const std::filesystem::path& selectedFile,
											 SceneFixture& fixture);

	static const std::filesystem::path& GetCurrentSceneFilepath() { return currentSceneSaveFile_; }

private:
	static Result<Void> SaveImpl(const std::filesystem::path& filepath,
								 const SceneFixture& fixture);

	static const std::filesystem::path& GetDefaultScenesFolder();

	static std::filesystem::path MakeDefaultSceneFilePath(size_t counter);
	static const std::string& GetDefaultScenesFolderString();
	
	static inline size_t defaultFileNameCounter_ = 0;
	static inline std::filesystem::path currentSceneSaveFile_{};
};





} // ui

#endif