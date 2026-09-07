#include "DirectoryViewerChild.h"

#if IMGUI_ENABLED

namespace ui {

namespace {

GuiTexture GetNodeIconTexture(const AssetViewerIcons& icons, const GuiTextureConverter& converter, 
							  const AssetReferenceNode& node, const AssetItem& item)
{
	switch (item.type)
	{
	case AssetItem::Type::Directory:
		return converter.FromSprite((node.isOpen
			? icons.folderOpenSmallSprite
			: icons.folderClosedSmallSprite));
	case AssetItem::Type::Image:
		return converter.FromSprite(icons.imageFileSmallSprite);
	case AssetItem::Type::Audio:
		return converter.FromSprite(icons.audioFileSmallSprite);
	case AssetItem::Type::Font:
		return converter.FromSprite(icons.fontFileSmallSprite);
	case AssetItem::Type::Script:
		return converter.FromSprite(icons.scriptFileSmallSprite);
	}

	return converter.FromSprite(icons.unknownFileSmallSprite);
}

} // unnamed

void DirectoryViewerChild::Draw(const AssetViewerIcons& icons, const GuiTextureConverter& converter)
{
	if (assetTree_.rootReferenceNode.id >= assetTree_.assets.size() ||
		!std::filesystem::exists(assetTree_.assets[assetTree_.rootReferenceNode.id].path))
	{
		return;
	}

	DrawImpl(assetTree_.rootReferenceNode, icons, converter, false);
}

void DirectoryViewerChild::DrawImpl(AssetReferenceNode& node, const AssetViewerIcons& icons,
									const GuiTextureConverter& converter, bool indent)
{
	assert(node.id < assetTree_.assets.size());
	auto& item = assetTree_.assets[node.id];

	ImGui::PushID(node.id);

	if (indent)
	{
		ImGui::Indent(ImGui::GetStyle().IndentSpacing * 0.7f);
	}

	const bool isLeaf = node.children.empty();
	assert(!isLeaf == (item.type == AssetItem::Type::Directory));

	const float arrowHeight = ImGui::GetFrameHeight();
	const float spacingX = ImGui::GetStyle().ItemSpacing.x;

	if (item.type == AssetItem::Type::Directory)
	{
		ImGui::BeginDisabled(isLeaf);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
			ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

		if (ImGui::ArrowButton("##arrowBtn",
			(node.isOpen && !isLeaf) ? ImGuiDir_Down : ImGuiDir_Right))
		{
			node.isOpen = !node.isOpen;
		}

		ImGui::PopStyleColor(3);

		ImGui::EndDisabled();
	}
	else
	{
		ImGui::Dummy(ImVec2(arrowHeight, 0));
	}

	ImGui::SameLine(0.0f, spacingX);

	if (ImGui::Selectable("##sel", false, 0,
		ImVec2(0.0f, arrowHeight - ImGui::GetStyle().FramePadding.y)))
	{

	}

	if (ImGui::BeginDragDropSource())
	{
		const auto payloadName = GetAssetItemPayloadName(item.type);
		if (!payloadName.empty())
		{
			ImGui::SetDragDropPayload(payloadName.data(), &node.id, sizeof(node.id));
		}

		ImGui::TextUnformatted(item.displayName.c_str());

		ImGui::EndDragDropSource();
	}

	const auto posMin = ImGui::GetItemRectMin();
	const auto posMax = ImGui::GetItemRectMax();
	const auto rowHeight = posMax.y - posMin.y;

	auto* drawList = ImGui::GetWindowDrawList();

	// asset icon
	auto tx = GetNodeIconTexture(icons, converter, node, item);
	assert(tx.textureId != 0);

	const float scale = std::min(
		rowHeight / tx.size.x,
		rowHeight / tx.size.y
	);

	const ImVec2 iconSize{
		tx.size.x * scale * 0.9f,
		tx.size.y * scale * 0.9f
	};

	const ImVec2 rowMin = ImGui::GetItemRectMin();

	const ImVec2 iconMin{
		rowMin.x + 4.0f,
		rowMin.y + (rowHeight - iconSize.y) * 0.5f
	};

	const ImVec2 iconMax{
		iconMin.x + iconSize.x,
		iconMin.y + iconSize.y
	};

	drawList->AddImage(
		tx.textureId,
		iconMin,
		iconMax,
		tx.uv0,
		tx.uv1
	);

	const auto displayNameTextSize = ImGui::CalcTextSize(item.displayName.c_str());
	const auto displayNameTextPos = ImVec2(iconMin.x + iconSize.x + spacingX,
		posMin.y + (rowHeight - ImGui::GetTextLineHeight()) * 0.5f);

	drawList->AddText(displayNameTextPos, ImGui::GetColorU32(ImGuiCol_Text),
		item.displayName.c_str());

	if (!isLeaf && node.isOpen)
	{
		for (auto& ch : node.children)
		{
			DrawImpl(ch, icons, converter, true);
		}
	}

	if (indent)
	{
		ImGui::Unindent(ImGui::GetStyle().IndentSpacing * 0.7f);
	}

	ImGui::PopID();
}

Result<DirectoryViewerChild> DirectoryViewerChild::Create(const std::filesystem::path& rootPath)
{
	if (!std::filesystem::exists(rootPath))
	{
		return MAKE_ERROR_FMT("Root path does not exist: '{}'", rootPath.string());
	}

	DirectoryViewerChild dirViewerChild{};

	dirViewerChild.assetTree_.Fill(rootPath);

	return dirViewerChild;
}

} // ui

#endif