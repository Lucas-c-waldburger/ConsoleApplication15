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

constexpr std::string_view kResourcesDirName = "resources";
constexpr std::string_view kAudioDirName = "audio";
constexpr std::string_view kMusicDirName = "music";
constexpr std::string_view kSoundsDirName = "sounds";
constexpr std::string_view kSpritesDirName = "sprites";
constexpr std::string_view kFontsDirName = "fonts";
constexpr std::string_view kScriptsDirName = "scripts";

template <typename...Args>
inline Result<std::string> JoinPaths(Args&&...args)
{
	fs::path fp = JoinPathsImpl(std::forward<Args>(args)...);
	std::string fpStr = fp.string();

	if (!fs::exists(fp))
	{
		return MAKE_ERROR_FMT("Path does not exist: '{}'", fpStr);
	}
	if (!fs::is_regular_file(fp))
	{
		return MAKE_ERROR_FMT("Path is not a regular file: '{}'", fpStr);
	}

	return fpStr;
}

template <typename...Args>
inline fs::path JoinPathsImpl(Args&&...args)
{
	return FilePathUtility::GetRootPath() / kResourcesDirName /
		(fs::path(std::forward<Args>(args)) / ...);
}

class ResourcePath
{
public:
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

	static Result<std::string> Font(std::string_view file)
	{
		return JoinPaths(kFontsDirName, file);
	}

	static Result<std::string> Script(std::string_view file)
	{
		return JoinPaths(kScriptsDirName, file);
	}

private:
	ResourcePath() = default;
};

template <typename Sorter>
concept PathStringSorter =
std::is_invocable_r_v<bool, Sorter, const std::string&, const std::string&>;

class ResourcePaths
{
public:
	using Ret = Result<std::vector<std::string>>;

	static Ret MusicDirectory(std::string_view dir)
	{
		return GetDirPaths(kAudioDirName, kMusicDirName, dir);
	}

	template <PathStringSorter Sorter>
	static Ret MusicDirectory(std::string_view dir, Sorter&& sorter)
	{
		TRY(GetDirPaths(kAudioDirName, kMusicDirName, dir), paths);
		std::sort(paths.begin(), paths.end(), std::forward<Sorter>(sorter));
		return paths;
	}

	static Ret SoundDirectory(std::string_view dir)
	{
		return GetDirPaths(kAudioDirName, kSoundsDirName, dir);
	}

	template <PathStringSorter Sorter>
	static Ret SoundDirectory(std::string_view dir, Sorter&& sorter)
	{
		TRY(GetDirPaths(kAudioDirName, kSoundsDirName, dir), paths);
		std::sort(paths.begin(), paths.end(), std::forward<Sorter>(sorter));
		return paths;
	}

	static Ret SpriteDirectory(std::string_view dir)
	{
		return GetDirPaths(kSpritesDirName, dir);
	}

	template <PathStringSorter Sorter>
	static Ret SpriteDirectory(std::string_view dir, Sorter&& sorter)
	{
		TRY(GetDirPaths(kSpritesDirName, dir), paths);
		std::sort(paths.begin(), paths.end(), std::forward<Sorter>(sorter));
		return paths;
	}

	static Ret FontDirectory(std::string_view dir)
	{
		return GetDirPaths(kFontsDirName, dir);
	}

	template <PathStringSorter Sorter>
	static Ret FontDirectory(std::string_view dir, Sorter&& sorter)
	{
		TRY(GetDirPaths(kFontsDirName, dir), paths);
		std::sort(paths.begin(), paths.end(), std::forward<Sorter>(sorter));
		return paths;
	}

	static Ret ScriptDirectory(std::string_view dir)
	{
		return GetDirPaths(kScriptsDirName, dir);
	}

	template <PathStringSorter Sorter>
	static Ret ScriptDirectory(std::string_view dir, Sorter&& sorter)
	{
		TRY(GetDirPaths(kScriptsDirName, dir), paths);
		std::sort(paths.begin(), paths.end(), std::forward<Sorter>(sorter));
		return paths;
	}

private:
	template <typename...Args>
	static Result<std::vector<std::string>> GetDirPaths(Args&&...args)
	{
		fs::path dir = JoinPathsImpl(std::forward<Args>(args)...);

		if (!fs::exists(dir))
		{
			return MAKE_ERROR_FMT("Path does not exist: '{}'", dir.string());
		}
		if (!fs::is_directory(dir))
		{
			return MAKE_ERROR_FMT("Path is not a directory: '{}'", dir.string());
		}

		std::vector<std::string> pathStrs;
		for (const auto& entry : fs::directory_iterator(dir))
		{
			if (fs::is_regular_file(entry))
			{
				pathStrs.emplace_back((dir / entry.path().filename()).string());
			}
		}

		return pathStrs;
	}

	ResourcePaths() = default;
};