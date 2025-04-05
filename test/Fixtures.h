#pragma once
#include <filesystem>
#include <format>
#include <cstdlib>
#include <thread>
#include "../ecs/Ecs.h"
#include "../scripting/ScriptManager.h"
#include "../core/Monitoring.h"
#include "../Hooks.h"
#include "Premades.h"

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
		static std::unique_ptr<ScriptFixture> PhysicsEditor(Entity& entity)
		{
			static constexpr const char* kPhysEditScriptName = "test::physics";
			static constexpr const char* kPhysEditScriptFile = "test_physics.lua";

			if (!entity.HasComponent<Physics>())
			{
				LOG_ERROR("Entity did not have physics component. Setup unsuccesful");
				return nullptr;
			}

			auto& phys = entity.GetComponent<Physics>();

			// TODO: make Lua class interfacey like ScriptManager to not make 
			// working with a single Lua a pain in the ass
			ScriptInstance instance{};

			instance.scriptInfo = {
				.name = kPhysEditScriptName,
				.scriptType = ScriptType::File,
				.path = std::format(kScriptsPathFmt, kPhysEditScriptFile)
			};

			instance.setupFn = [&phys](Lua& lua) {
				lua["physics"] = &phys;
			};  

			auto fixture = std::make_unique<ScriptFixture>();

			fixture->scriptName_ = kPhysEditScriptName;
			fixture->fileMonitor_.SetFilePath(instance.scriptInfo.path);
			fixture->lua_.SetScriptInfo(std::move(instance.scriptInfo));
			
			instance.setupFn(fixture->lua_);

			HookManager::EnableHooks(HookPoint::PrePhysicsUpdate);

			HookManager::Attach<HookPoint::PrePhysicsUpdate>(kPhysEditScriptName,
				[&fixture]() { assert(fixture); RunOnFileChange(*fixture); });

			OpenInVsCode(fixture->scriptName_);

			return fixture;
		}

	private:
		GetInstance() = default;

		static void RunOnFileChange(ScriptFixture& fixture)
		{
			if (fixture.fileMonitor_.FileDidChange())
			{
				LOG_IF_ERROR(fixture.lua_.Run());
			}
		}
	};


	ScriptFixture() = default;
	~ScriptFixture()
	{
		HookManager::Detach(hookedPoint_, scriptName_);
	}

private:
	static void OpenInVsCode(std::string_view scriptName)
	{
		namespace fs = std::filesystem;

		std::string scriptPath = std::format(kScriptsPathFmt, scriptName);
		if (!fs::exists(scriptPath))
		{
			LOG_WARNING_FMT("No script with name '{}' found inside scripts directory");
			return;
		}

		std::string command = std::format(kVsCodePathFmt, scriptPath);

		system(command.c_str());
	}


	//static ScriptManager& GetScriptManager()
	//{
	//	static std::unique_ptr<ScriptManager> instance;
	//	if (!instance)
	//	{
	//		instance = std::unique_ptr<ScriptManager>(new ScriptManager{});
	//	}

	//	return *instance;
	//}
	//ScriptManager scriptManager_;
	Lua lua_;
	FileChangeMonitor fileMonitor_;
	HookPoint hookedPoint_;
	std::string scriptName_;
};









