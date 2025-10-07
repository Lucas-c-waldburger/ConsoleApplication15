#include "FilePathUtility.h"
#include <cassert>

std::filesystem::path FilePathUtility::rootPath_{};

void FilePathUtility::Init(const char* argv0, Root root)
{
	if (rootPath_.empty())
	{
		rootPath_ = fs::absolute(argv0).parent_path();

		switch (root)
		{
		case Root::Exe:
			break;
		case Root::Proj: default:
			rootPath_ = rootPath_.parent_path();
			break;
		}

		assert(fs::is_directory(rootPath_));
	}
}

const fs::path& FilePathUtility::GetRootPath()
{
	if (rootPath_.empty())
	{
		rootPath_ = fs::current_path();
	}
	return rootPath_;
}
