#include "SaveSceneUtility.h"

#if IMGUI_ENABLED
#include "../../systems/SerializationSystem.h"
#include "../../file/FilePathUtility.h"
#include "../../deps/tinyfiledialogs/tinyfiledialogs.h"
#include "../../serial/SerializationUtils.h"
#include "../../sdl/SDLite.h"
#include "../Fixtures.h"

namespace ui {

namespace fs = std::filesystem;

namespace {

//constexpr const char* ToString(FileDialog::IconType iconType)
//{
//	switch (iconType)
//	{
//	case FileDialog::IconType::Info: return "info";
//	case FileDialog::IconType::Error: return "error";
//	case FileDialog::IconType::Warning: return "warning";
//	case FileDialog::IconType::Question: return "question";
//	}
//	return "";
//}


} // unnamed

//void FileDialog::NotifyPopup(std::string_view title, std::string_view msg, IconType iconType)
//{
//	tinyfd_notifyPopup(title.data(), msg.data(), ToString(iconType));
//}

Result<Void> SceneSaveUtility::HandleSceneSave(const SceneFixture& fixture)
{
	if (!fs::exists(currentSceneSaveFile_))
	{
		return HandleSceneSaveAs(fixture);
	}

	return SaveImpl(currentSceneSaveFile_, fixture);
}

Result<Void> SceneSaveUtility::HandleSceneSaveAs(const SceneFixture& fixture)
{
	auto defaultFilepath = MakeDefaultSceneFilePath(defaultFileNameCounter_);
	if (fs::exists(defaultFilepath))
	{
		defaultFilepath = MakeDefaultSceneFilePath(++defaultFileNameCounter_);
		assert(!fs::exists(defaultFilepath));
	}

	const std::string defaultFilepathStr = defaultFilepath.string();

	static constexpr const char* filters[] = { "*.json" };

	const char* selectedFile = tinyfd_saveFileDialog(
		"Save Scene",
		defaultFilepathStr.c_str(),
		1,
		filters,
		".json"
	);

	if (!selectedFile)
	{
		return kVoid;
	}

	currentSceneSaveFile_ = fs::path(selectedFile);

	return SaveImpl(currentSceneSaveFile_, fixture);
}

std::optional<fs::path> SceneSaveUtility::QuerySceneOpen()
{
	static constexpr const char* filters[] = { "*.json" };

	const char* selectedFile = tinyfd_openFileDialog(
		"Load Scene",
		GetDefaultScenesFolderString().c_str(),
		1,
		filters,
		".json",
		0
	);

	if (!selectedFile)
	{
		return std::nullopt;
	}

	auto path = fs::path(selectedFile);
	assert(fs::exists(path));

	return path;
}

SceneSaveUtility::SceneOpenResponse 
SceneSaveUtility::HandleSceneOpen(const fs::path& selectedFile, SceneFixture& fixture)
{
	SceneSaveUtility::SceneOpenResponse response{};

	auto jResult = LoadJson(selectedFile.string());
	if (!jResult.Success())
	{
		response.errors.emplace_back(std::move(jResult).GetError());

		return response;
	}

	currentSceneSaveFile_ = fs::path(selectedFile);
	assert(fs::exists(currentSceneSaveFile_));

	response.errors = fixture.DeserializeStateFromJson(jResult.GetValue());
	response.success = true;

	return response;
}

Result<Void> SceneSaveUtility::SaveImpl(const std::filesystem::path& filepath,
										const SceneFixture& fixture)
{
	std::ofstream file(filepath);
	if (!file)
	{
		return MAKE_ERROR_FMT("Could not open save scene file '{}'", filepath.string());
	}

	nlohmann::json j;

	fixture.SerializeStateToJson(j);

	file << j.dump(4);

	return kVoid;
}

const std::filesystem::path& SceneSaveUtility::GetDefaultScenesFolder()
{
	static const std::filesystem::path kDefaultScenesFolder =
		FilePathUtility::GetRootPath() / "test" / "ui" / "scenes";

	return kDefaultScenesFolder;
}

std::filesystem::path SceneSaveUtility::MakeDefaultSceneFilePath(size_t counter)
{
	return (GetDefaultScenesFolder() / std::format("new_scene_{}.json", counter));
}

const std::string& SceneSaveUtility::GetDefaultScenesFolderString()
{
	static const std::string kDefaultScenesFolderString = (GetDefaultScenesFolder() / "").string();

	return kDefaultScenesFolderString;
}

} // ui

#endif