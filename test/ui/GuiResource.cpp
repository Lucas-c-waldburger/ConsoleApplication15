#include "GuiResource.h"

#if IMGUI_ENABLED
#include "../../file/FilePathUtility.h"

namespace ui {

namespace {

Result<GuiFonts> LoadGuiFonts()
{
	TRY(ResourcePath::Font("Inter/Inter_18pt-Regular.ttf"), regFontPath);
	TRY(ResourcePath::Font("Inter/Inter_18pt-Medium.ttf"), medFontPath);
	TRY(ResourcePath::Font("Inter/Inter_18pt-Thin.ttf"), thinFontPath);
	TRY(ResourcePath::Font("Inter/Inter_18pt-Bold.ttf"), boldFontPath);
	TRY(ResourcePath::Font("Inter/Inter_18pt-SemiBold.ttf"), semiBoldFontPath);

	ImFontConfig cfg;
	cfg.OversampleH = 2;
	cfg.OversampleV = 2;

	ImGuiIO& io = ImGui::GetIO();

	GuiFonts fonts{};

	auto load = [&](auto& fontMember, const auto& path) {
		fontMember = io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f, &cfg);
		assert(fontMember != nullptr);
	};

	load(fonts.regular, regFontPath);
	load(fonts.medium, medFontPath);
	load(fonts.thin, thinFontPath);
	load(fonts.bold, boldFontPath);
	load(fonts.semiBold, semiBoldFontPath);

	io.Fonts->Build();

	io.FontDefault = fonts.regular;

	return fonts;
}

} // unnamed

Result<Void> GuiResource::Init()
{
	if (fonts_.regular != nullptr)
	{
		return MAKE_ERROR("GuiResource already initialized - Fonts previously loaded");
	}

	TRY_ASSIGN(fonts_, LoadGuiFonts());

	return kVoid;
}


} // ui

#endif