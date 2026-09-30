#pragma once
#include "../ui/gui_lua_api/GuiLuaApiRegistration.h"
#include "../Fixtures.h"
#include "../../core/Monitoring.h"
#include <sol/sol.hpp>

#if IMGUI_ENABLED

namespace test {

class GuiLuaPlayground
{
public:
	Result<Void> Init(const std::string& scriptPath, SceneFixture& fixture);

private:
	Result<Void> LoadScript();
	Result<Void> RunDrawFunction();

	sol::state state_;
	sol::function drawFn_;
	FileChangeMonitor fileChangeMonitor_;
	bool failed_ = false;
};


} // test

#endif