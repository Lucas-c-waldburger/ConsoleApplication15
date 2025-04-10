#include "Fixtures.h"
#include "../ecs/Ecs.h"

std::unique_ptr<ScriptFixture> ScriptFixture::GetInstance::PhysicsEditor(Entity& entity)
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

	auto& hookPoint = Hooks::GetHookPoint(HookPoint::PrePhysicsUpdate);
	assert(hookPoint);

	fixture->hookPointIdent_ = HookPoint::PrePhysicsUpdate;
	fixture->attachmentHandle_ = hookPoint->Attach([&fixture]() { 
		assert(fixture); 
		RunOnFileChange(*fixture); 
	});

	OpenInVsCode(fixture->scriptName_);

	return fixture;
}

void ScriptFixture::GetInstance::RunOnFileChange(ScriptFixture& fixture)
{
	if (fixture.fileMonitor_.FileDidChange())
	{
		LOG_IF_ERROR(fixture.lua_.Run());
	}
}

void ScriptFixture::TearDown()
{
	auto& hookPoint = Hooks::GetHookPoint(hookPointIdent_);
	if (hookPoint)
	{
		hookPoint->Detach(attachmentHandle_);
	}
}

void ScriptFixture::OpenInVsCode(std::string_view scriptName)
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
