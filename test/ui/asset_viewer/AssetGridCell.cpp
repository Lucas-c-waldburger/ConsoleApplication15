#include "AssetGridCell.h"

#if IMGUI_ENABLED

namespace ui {

void AssetGridCell::DrawThumbnailTexture(const GuiTexture& tx) const
{
	if (tx.textureId == 0)
	{
		return;
	}

	const float txScale = std::min(
		kThumbnailTextureSize / tx.size.x,
		kThumbnailTextureSize / tx.size.y
	);

	const ImVec2 txSize{
		tx.size.x * txScale,
		tx.size.y * txScale
	};

	const float thumbnailX = min.x + (width - kThumbnailTextureSize) * 0.5f;

	const ImVec2 thumbnailMin{
		thumbnailX,
		min.y
	};

	const ImVec2 thumbnailMax{
		thumbnailX + kThumbnailTextureSize,
		min.y + kThumbnailTextureSize
	};

	ImGui::GetWindowDrawList()->AddImage(
		tx.textureId,
		thumbnailMin,
		thumbnailMax,
		tx.uv0,
		tx.uv1
	);
}

void AssetGridCell::DrawDisplayText(std::string_view text) const
{
	const ImVec2 textPos{
		min.x,
		min.y + kThumbnailTextureSize + 4.0f
	};

	ImGui::GetWindowDrawList()->AddText(
		textPos,
		ImGui::GetColorU32(ImGuiCol_Text),
		text.data()
	);
}

void AssetGridCell::DrawSelectedHighlight() const
{
	ImGui::GetWindowDrawList()->AddRect(
		min,
		max,
		ImGui::GetColorU32(ImGuiCol_Header),
		2.0f,
		0,
		2.0f
	);
}

bool AssetGridCell::Clicked() const
{
	return (interaction & InteractionState::Hovered) &&
		(interaction & InteractionState::AnyClick);
}

AssetGridCell AssetGridCell::Place()
{
	const float cellHeight = width + 4.0f + ImGui::GetTextLineHeightWithSpacing();

	ImGui::InvisibleButton("##assetGridCell", ImVec2(width, cellHeight));

	AssetGridCell cell{
		.height = cellHeight,
		.min = ImGui::GetItemRectMin(),
		.max = ImGui::GetItemRectMax()
	};

	auto* drawList = ImGui::GetWindowDrawList();

	if (ImGui::IsItemHovered())
	{
		drawList->AddRectFilled(
			cell.min,
			cell.max,
			ImGui::GetColorU32(ImGuiCol_FrameBgHovered)
		);

		cell.interaction = InteractionState::Hovered;

		if (ImGui::IsMouseClicked(0))
		{
			if (ImGui::IsMouseDoubleClicked(0))
			{
				cell.interaction |= InteractionState::ClickDouble;
			}
			else
			{
				cell.interaction |= InteractionState::ClickSingle;
			}
		}
	}

	return cell;
}

} // ui

#endif