#pragma once
#include <filesystem>
#include "../core/Result.h"

namespace fs = std::filesystem;

class FilePathUtility
{
public:
	enum class Root
	{
		Proj,
		Exe
	};

	static void Init(const char* argv0, Root root = Root::Proj);
	static const fs::path& GetRootPath();

private:
	static fs::path rootPath_;
};

class ResourcePath
{
public:
	static constexpr std::string_view kResourcesDirName = "resources";
	static constexpr std::string_view kAudioDirName = "audio";
	static constexpr std::string_view kMusicDirName = "music";
	static constexpr std::string_view kSoundsDirName = "sounds";
	static constexpr std::string_view kSpritesDirName = "sprites";
	static constexpr std::string_view kScriptsDirName = "scripts";

	//static constexpr char kAudioDirName[] = "audio";
	//static constexpr char kMusicDirName[] = "music";
	//static constexpr char kSoundsDirName[] = "sounds";
	//static constexpr char kSpritesDirName[] = "sprites";
	//static constexpr char kScriptsDirName[] = "scripts";

	static Result<std::string> Music(std::string_view file)
	{
		return JoinPaths(kAudioDirName, kMusicDirName, file);
	}

	static Result<std::string> Sound(std::string_view file)
	{
		return JoinPaths(kAudioDirName, kSoundsDirName, file);
	}

	static Result<std::string> Sprite(std::string_view file)
	{
		return JoinPaths(kSpritesDirName, file);
	}

	static Result<std::string> Script(std::string_view file)
	{
		return JoinPaths(kScriptsDirName, file);
	}

private:
	//template <const char*...subDirs>
	//static Result<std::string> JoinPaths(std::string_view file)
	//{
	//	fs::path fp =
	//		FilePathUtility::GetExePath() / JoinPathsImpl<subDirs...>();
	//	//(fp /= ... / subDirs);  
	//	fp /= file;

	//	if (!fs::exists(fp))
	//	{
	//		return MAKE_ERROR_FMT("Path invalid: '{}'", fp.string());
	//	}
	//	if (!fs::is_regular_file(fp))
	//	{
	//		return MAKE_ERROR_FMT("Path is not a regular file: '{}'", fp.string());
	//	}

	//	return fp.string();
	//}

	template <typename...Args>
	static Result<std::string> JoinPaths(Args&&...args)
	{
		fs::path fp = JoinPathsImpl(std::forward<Args>(args)...);
		std::string fpStr = fp.string();

		if (!fs::exists(fp))
		{
			return MAKE_ERROR_FMT("Path invalid: '{}'", fpStr);
		}
		if (!fs::is_regular_file(fp))
		{
			return MAKE_ERROR_FMT("Path is not a regular file: '{}'", fpStr);
		}

		return fpStr;
	}

	template <typename...Args>
	static fs::path JoinPathsImpl(Args&&...args)
	{
		return FilePathUtility::GetRootPath() / kResourcesDirName / 
			(fs::path(std::forward<Args>(args)) / ...);
	}
};
