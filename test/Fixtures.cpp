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
	ScriptInstance scriptInstance{};

	scriptInstance.scriptInfo = {
		.name = kPhysEditScriptName,
		.scriptType = ScriptType::File,
		.path = std::format(kScriptsPathFmt, kPhysEditScriptFile)
	};

	scriptInstance.setupFn = [&phys](Lua& lua) {
		lua["physics"] = &phys;
	};

	auto fixture = std::unique_ptr<ScriptFixture>(new ScriptFixture{});

	fixture->scriptName_ = kPhysEditScriptName;
	fixture->fileMonitor_.SetFilePath(scriptInstance.scriptInfo.path);

	fixture->lua_.SetScriptInfo(std::move(scriptInstance.scriptInfo));
	scriptInstance.setupFn(fixture->lua_);

	auto& hookPoint = Hooks::GetHookPoint(HookPoint::PrePhysicsUpdate);
	assert(hookPoint);

	fixture->hookPointIdent_ = HookPoint::PrePhysicsUpdate;
	fixture->attachmentHandle_ = hookPoint->Attach([&fixture]() { 
		assert(fixture); 
		RunOnFileChange(*fixture); 
	});

	OpenInVsCode(kPhysEditScriptFile);

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

	//fileMonitor_.
}

void ScriptFixture::OpenInVsCode(std::string_view scriptName)
{
	namespace fs = std::filesystem;

	std::string scriptPath = std::format(kScriptsPathFmt, scriptName);

	fs::path source = __FILE__;
	auto base = source.parent_path().parent_path();
	base /= scriptPath;

	//std::string scriptPath = std::format(kScriptsPathFmt, scriptName);
	if (!fs::exists(base))
	{
		LOG_WARNING_FMT("No script with name '{}' found inside scripts directory", scriptName);
		return;
	}

	std::string command = std::format(kVsCodePathFmt, base.string());

	system(command.c_str());
}
