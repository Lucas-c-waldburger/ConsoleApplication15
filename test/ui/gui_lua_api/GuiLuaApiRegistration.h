#pragma once
#include "GuiLuaApiCommon.h"

#if IMGUI_ENABLED

class TextureRepository;

namespace ui {

void GuiLuaRegisterStandaloneFunctions(sol::table& tbl);

struct GuiTextureSources
{
	const TextureRepository& uiTextures;
	const TextureRepository& gameTextures;
};

void GuiLuaRegisterTextureFunctions(sol::table& tbl, const GuiTextureSources& txSrcs);


void GuiLuaRegister(sol::state_view state, const GuiTextureSources& txSrcs);


} // ui

#endif