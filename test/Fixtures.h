#pragma once
#include <format>
#include <cstdlib>
#include <thread>
#include "../scripting/ScriptManager.h"
#include "../core/Monitoring.h"

class Entity;

class ScriptFixture
{
public:
	static constexpr std::string_view kVsCodePathFmt = 
		R"(C:\Users\Lucas\AppData\Local\Programs\Microsoft VS Code\Code.exe\ {})";
	static constexpr std::string_view kScriptsPathFmt = R"(..\..\resources\scripts\{})";

	friend class GetInstance;

	class GetInstance
	{
	public:
		static std::unique_ptr<ScriptFixture> PhysicsEditor(Entity& entity);

	private:
		GetInstance() = default;

		static void RunOnFileChange(ScriptFixture& fixture);
	};

	const std::string& GetScriptName() const { return scriptName_; }

private:
	ScriptFixture() = default;

	static void OpenInVsCode(std::string_view scriptName);

	Lua lua_;
	FileChangeMonitor fileMonitor_;
	std::string scriptName_;
};









