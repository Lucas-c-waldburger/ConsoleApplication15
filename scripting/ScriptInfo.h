#pragma once
#include <string>

enum class ScriptType
{
	Unknown,
	String,
	File
};

struct ScriptInfo
{
	std::string name;
	ScriptType scriptType = ScriptType::Unknown;
	std::string path;
};