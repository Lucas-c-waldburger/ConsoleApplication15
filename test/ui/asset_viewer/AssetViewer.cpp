#include "AssetViewer.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"
#include "../../../file/FilePathUtility.h"
#include <imgui_internal.h>

namespace ui {

bool AssetViewer::Draw(SceneFixture& fixture)
{
	auto& primaryRepo = fixture.GetTextureRepository();
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return false;
	}

	GuiTextureConverter loadTargetConverter{ primaryRepo };
	GuiTextureConverter uiTexturesConverter{ *auxRepo };

	ImGui::SetNextWindowPos(
		ImGui::GetMainViewport()->WorkPos,
		ImGuiCond_FirstUseEver
	);

	bool isOpen = true;

	if (!ImGui::Begin("Assets", &isOpen))
	{
		ImGui::End();

		return isOpen;
	}

	const ImVec2 available = ImGui::GetContentRegionAvail();

	constexpr float bottomHeight = 200.0f;
	constexpr float leftWidth = 220.0f;
	constexpr float spacing = 4.0f;

	const float topHeight = available.y - bottomHeight - spacing;

	auto* g = ImGui::GetCurrentContext();
	const float resizeHitbox = g->WindowsBorderHoverPadding;
	g->WindowsBorderHoverPadding = resizeHitbox * 2.0f;

	if (ImGui::BeginChild("AssetBrowser", ImVec2(available.x, topHeight),
		ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
	{
		// Directories //
		ImGui::BeginChild("Directories", ImVec2(leftWidth, 0.0f),
			(ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX));

		directoryViewer_.Draw(icons_, uiTexturesConverter);

		ImGui::EndChild();

		ImGui::SameLine(0.0f, spacing);

		// Asset Grid //
		ImGui::BeginChild("AssetGrid", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

		AssetGridViewerChild::ResourceContext ctx{
			.assetTree = directoryViewer_.GetAssetTree(),
			.icons = icons_,
			.uiTexturesConverter = uiTexturesConverter
		};

		assetGridViewer_.Draw(fixture, ctx);

		ImGui::EndChild();

		assetGridViewer_.HandleAssetDragDropTarget(fixture, ctx);
	}

	ImGui::EndChild();

	g->WindowsBorderHoverPadding = resizeHitbox;

	// Sprite Viewer //
	ImGui::BeginChild("SpriteViewer", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

	AssetPreviewViewerChild::ResourceContext ctx{
		.spriteSelection = assetGridViewer_.GetSpriteSelection(),
		.loadTargetConverter = loadTargetConverter,
		.uiTexturesConverter = uiTexturesConverter
	};

	assetPreviewViewer_.Draw(fixture, ctx);

	ImGui::EndChild();

	ImGui::End();

	return isOpen;
}

Result<Void> AssetViewer::Init(SceneFixture& fixture)
{
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Auxilliary texture repository was null");
	}

	TRY_ASSIGN(icons_, AssetViewerIcons::Load(fixture.GetRenderer(), auxRepo->GetSpriteAtlas()));

	TRY(ResourcePath::Root(), rootResourcePath);
	TRY_ASSIGN(directoryViewer_, DirectoryViewerChild::Create(rootResourcePath));

	TRY(assetPreviewViewer_.Init(fixture));

	return kVoid;
}

} // ui

#endif