#include "AssetViewer.h"

#if IMGUI_ENABLED
#include "../GuiMouse.h"
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

	bool isOpen = true;

	if (!ImGui::Begin(GetEditorWindowName(EditorWindowType::AssetWindow).data(), &isOpen))
	{
		ImGui::End();

		return isOpen;
	}

	GuiMouse::EvaluateInsideWindow(EditorWindowType::AssetWindow);

	const ImVec2 available = ImGui::GetContentRegionAvail();

	constexpr float bottomHeight = 200.0f;
	constexpr float leftWidth = 220.0f;
	constexpr float spacing = 4.0f;

	const float topHeight = available.y - bottomHeight - spacing;

	auto* g = ImGui::GetCurrentContext();
	const float resizeHitbox = g->WindowsBorderHoverPadding;
	g->WindowsBorderHoverPadding = resizeHitbox * 2.0f;

	DataRecord<AssetItem::Type> openGridTabTypeRecord{
		.last = assetGridViewer_.GetOpenAssetTabType()
	};

	ImGui::BeginChild("AssetBrowser", ImVec2(available.x, topHeight),
		ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY);

	GuiMouse::EvaluateInsideWindow(EditorWindowType::AssetWindow);
	
	// Directories //
	const bool wasExpanded = dirChildInfo_.expanded;

	ImGui::BeginChild("Directories",ImVec2(
		wasExpanded ? dirChildInfo_.expandWidth : 
					  DirectoryViewerChildInfo::kCollapsedWidth,
		0.0f),
		ImGuiChildFlags_Borders |
		(wasExpanded ? ImGuiChildFlags_ResizeX : 0));

	GuiMouse::EvaluateInsideWindow(EditorWindowType::AssetWindow);

	const bool isExpanded = directoryViewer_.Draw(icons_, uiTexturesConverter);
	if (wasExpanded)
	{
		dirChildInfo_.expandWidth = ImGui::GetWindowWidth();
	}

	if (isExpanded != wasExpanded)
	{
		ImGui::SetWindowSize(ImVec2(
			isExpanded ? dirChildInfo_.expandWidth : 
						 DirectoryViewerChildInfo::kCollapsedWidth,
			ImGui::GetWindowHeight()),
			ImGuiCond_Always);
	}

	dirChildInfo_.expanded = isExpanded;

	ImGui::EndChild();

	ImGui::SameLine(0.0f, spacing);

	// Asset Grid //
	ImGui::BeginChild("AssetGrid", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

	GuiMouse::EvaluateInsideWindow(EditorWindowType::AssetWindow);

	AssetGridViewerChild::ResourceContext gridCtx{
		.assetTree = directoryViewer_.GetAssetTree(),
		.icons = icons_,
		.uiTexturesConverter = uiTexturesConverter
	};

	assetGridViewer_.Draw(fixture, gridCtx);

	ImGui::EndChild();

	assetGridViewer_.HandleAssetDragDropTarget(fixture, gridCtx);

	ImGui::EndChild();

	g->WindowsBorderHoverPadding = resizeHitbox;

	// Asset Preview Viewer //
	ImGui::BeginChild("PreviewViewerDockArea", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

	ImGuiID dockspaceId = ImGui::GetID("PreviewViewerDockSpace");

	if (!ImGui::DockBuilderGetNode(dockspaceId))
	{
		ImGui::DockBuilderAddNode(
			dockspaceId,
			ImGuiDockNodeFlags_DockSpace);

		ImGui::DockBuilderSetNodeSize(
			dockspaceId,
			ImGui::GetContentRegionAvail());

		ImGui::DockBuilderDockWindow(
			"PreviewViewer",
			dockspaceId);

		ImGui::DockBuilderFinish(dockspaceId);
	}

	ImGui::DockSpace(dockspaceId, ImGui::GetContentRegionAvail(),
		(ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingSplit));

	ImGui::EndChild(); 

	ImGui::End();

	ImGui::Begin("PreviewViewer", nullptr);

	GuiMouse::EvaluateInsideWindow(EditorWindowType::AssetWindow);

	openGridTabTypeRecord.now = assetGridViewer_.GetOpenAssetTabType();

	AssetPreviewViewerChild::ResourceContext previewCtx{
		.spriteSelection = assetGridViewer_.GetSpriteSelection(),
		.audioSelection = assetGridViewer_.GetAudioSelection(),
		.fontSelection = assetGridViewer_.GetFontSelection(),
		.scriptSelection = assetGridViewer_.GetScriptSelection(),
		.icons = icons_,
		.loadTargetConverter = loadTargetConverter,
		.uiTexturesConverter = uiTexturesConverter
	};

	assetPreviewViewer_.Draw(fixture, previewCtx, openGridTabTypeRecord);

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

void AssetViewer::TearDown()
{
	assetPreviewViewer_.TearDown();
}

} // ui

#endif