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
constexpr std::string_view kJsonDirName = "json";

template <typename...Args>
inline fs::path JoinPathsRaw(Args&&...args)
{
	return FilePathUtility::GetRootPath() / kResourcesDirName /
		(fs::path(std::forward<Args>(args)) / ...);
}

template <typename...Args>
inline Result<std::string> JoinPaths(Args&&...args)
{
	fs::path fp = JoinPathsRaw(std::forward<Args>(args)...);
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

using ResourcePathResult = Result<std::string>;

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

	static Result<std::string> Json(std::string_view file)
	{
		return JoinPaths(kJsonDirName, file);
	}

private:
	ResourcePath() = default;
};

template <typename T, typename Sorter>
concept SomeSorter = std::is_invocable_r_v<bool, Sorter, const T&, const T&>;

template <typename Sorter>
concept SomeStringSorter = SomeSorter<std::string, Sorter>;

template <typename Sorter>
concept SomeFsPathSorter = SomeSorter<fs::path, Sorter>;

template <typename Sorter>
concept SomeResourcePathSorter = (SomeStringSorter<Sorter> || SomeFsPathSorter<Sorter>);

class ResourcePaths
{
public:
	using Ret = Result<std::vector<std::string>>;
	using DefaultSort = std::less<std::string>;

	static Ret MusicDirectory(std::string_view dir, bool defaultSort = true)
	{
		return (defaultSort)
			? SortAndReturnPaths(DefaultSort{}, kAudioDirName, kMusicDirName, dir)
			: GetDirPaths<std::string>(kAudioDirName, kMusicDirName, dir);
	}

	template <SomeResourcePathSorter Sorter>
	static Ret MusicDirectory(std::string_view dir, Sorter&& sorter)
	{
		return SortAndReturnPaths(std::forward<Sorter>(sorter), 
								  kAudioDirName, kMusicDirName, dir);
	}

	static Ret SoundDirectory(std::string_view dir, bool defaultSort = true)
	{
		return (defaultSort)
			? SortAndReturnPaths(DefaultSort{}, kAudioDirName, kSoundsDirName, dir)
			: GetDirPaths<std::string>(kAudioDirName, kSoundsDirName, dir);
	}

	template <SomeStringSorter Sorter>
	static Ret SoundDirectory(std::string_view dir, Sorter&& sorter)
	{
		return SortAndReturnPaths(std::forward<Sorter>(sorter),
								  kAudioDirName, kSoundsDirName, dir);
	}

	static Ret SpriteDirectory(std::string_view dir, bool defaultSort = true)
	{
		return (defaultSort)
			? SortAndReturnPaths(DefaultSort{}, kSpritesDirName, dir)
			: GetDirPaths<std::string>(kSpritesDirName, dir);		
	}

	template <SomeStringSorter Sorter>
	static Ret SpriteDirectory(std::string_view dir, Sorter&& sorter)
	{
		return SortAndReturnPaths(std::forward<Sorter>(sorter), kSpritesDirName, dir);
	}

	static Ret FontDirectory(std::string_view dir, bool defaultSort = true)
	{
		return (defaultSort)
			? SortAndReturnPaths(DefaultSort{}, kFontsDirName, dir)
			: GetDirPaths<std::string>(kFontsDirName, dir);
	}

	template <SomeStringSorter Sorter>
	static Ret FontDirectory(std::string_view dir, Sorter&& sorter)
	{
		return SortAndReturnPaths(std::forward<Sorter>(sorter), kFontsDirName, dir);
	}

	static Ret ScriptDirectory(std::string_view dir, bool defaultSort = true)
	{
		return (defaultSort)
			? SortAndReturnPaths(DefaultSort{}, kScriptsDirName, dir)
			: GetDirPaths<std::string>(kScriptsDirName, dir);
	}

	template <SomeStringSorter Sorter>
	static Ret ScriptDirectory(std::string_view dir, Sorter&& sorter)
	{
		return SortAndReturnPaths(std::forward<Sorter>(sorter), kScriptsDirName, dir);

	}

private:
	template <typename T, typename...Args> requires (std::same_as<T, std::string> ||
													 std::same_as<T, fs::path>)
	static Result<std::vector<T>> GetDirPaths(Args&&...args)
	{
		fs::path dir = JoinPathsRaw(std::forward<Args>(args)...);

		if (!fs::exists(dir))
		{
			return MAKE_ERROR_FMT("Path does not exist: '{}'", dir.string());
		}
		if (!fs::is_directory(dir))
		{
			return MAKE_ERROR_FMT("Path is not a directory: '{}'", dir.string());
		}

		std::vector<T> paths;
		for (const auto& entry : fs::directory_iterator(dir))
		{
			if (fs::is_regular_file(entry))
			{
				if constexpr (std::same_as<T, std::string>)
				{
					paths.emplace_back((dir / entry.path().filename()).string());
				}
				else
				{
					paths.emplace_back((dir / entry.path()));
				}
			}
		}

		return paths;
	}

	template <typename T, typename Sorter, typename...Args> 
		requires (SomeSorter<T, Sorter> && (std::same_as<T, std::string> || 
										   std::same_as<T, fs::path>))
	static Result<std::vector<std::string>> 
	SortAndReturnPathsImpl(Sorter&& sorter, Args&&...args)
	{
		TRY(GetDirPaths<T>(std::forward<Args>(args)...), paths);

		std::sort(paths.begin(), paths.end(), std::forward<Sorter>(sorter));
		
		if constexpr (std::same_as<T, fs::path>)
		{
			std::vector<std::string> pathStrs{ paths.size() };
			
			std::transform(paths.begin(), paths.end(), pathStrs.begin(), &fs::path::string);

			return pathStrs;
		}
		else
		{
			return paths;
		}
	}

	template <typename Sorter, typename...Args>
		requires SomeResourcePathSorter<Sorter>
	static Result<std::vector<std::string>> 
	SortAndReturnPaths(Sorter&& sorter, Args&&...args)
	{
		if constexpr (SomeSorter<fs::path, Sorter>)
		{
			return SortAndReturnPathsImpl<fs::path>(
				std::forward<Sorter>(sorter),
				std::forward<Args>(args)...);
		}
		else
		{
			return SortAndReturnPathsImpl<std::string>(
				std::forward<Sorter>(sorter),
				std::forward<Args>(args)...);
		}
	}

	ResourcePaths() = default;
};