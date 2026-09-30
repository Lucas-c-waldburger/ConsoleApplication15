#include "GuiLuaPlayground.h"

#if IMGUI_ENABLED
#include "../ui/asset_viewer/AssetViewerCommon.h"

namespace test {

Result<Void> GuiLuaPlayground::Init(const std::string& scriptName, SceneFixture& fixture)
{
	TRY(ResourcePath::Script(scriptName), path);

	fileChangeMonitor_.SetFilePath(path);

	state_.open_libraries(sol::lib::base);
	
	TRY(LoadScript());

	auto& repo = fixture.GetTextureRepository();
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Auxiliary texture repo was null");
	}

	TRY(ui::AssetViewerIcons::Load(fixture.GetRenderer(), auxRepo->GetSpriteAtlas()));

	ui::GuiLuaRegister(state_, { .uiTextures = *auxRepo, .gameTextures = repo });

	if (!fixture.IsSystemRegistered<GuiSystem>())
	{
		return MAKE_ERROR("Gui system not registered");
	}

	fixture.GetSystem<GuiSystem>().SetUI([this](float) {
		if (fileChangeMonitor_.FileDidChange())
		{
			LOG_IF_ERROR(LoadScript());
		}

		if (failed_)
		{
			return;
		}

		LOG_IF_ERROR(RunDrawFunction());
	});

	return kVoid;
}

Result<Void> GuiLuaPlayground::LoadScript()
{
	sol::protected_function_result res = state_.script_file(fileChangeMonitor_.GetFilePath());
	if (!res.valid())
	{
		failed_ = true;

		sol::error err = res;

		return MAKE_ERROR(err.what());
	}

	drawFn_ = state_["draw"];
	if (!drawFn_.valid())
	{
		failed_ = true;

		return MAKE_ERROR("No function named 'draw' in script");
	}

	failed_ = false;

	return kVoid;
}

Result<Void> GuiLuaPlayground::RunDrawFunction()
{
	if (!drawFn_.valid())
	{
		return kVoid;
	}

	sol::protected_function_result res = drawFn_();
	if (!res.valid())
	{
		failed_ = true;

		sol::error err = res;

		return MAKE_ERROR(err.what());
	}

	failed_ = false;

	return kVoid;
}

} // test

#endif