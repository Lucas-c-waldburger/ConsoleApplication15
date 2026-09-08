#include "AssetGridCell.h"

#if IMGUI_ENABLED
#include <misc/cpp/imgui_stdlib.h>

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

	const float centerX = min.x + width * 0.5f;
	const float centerY = min.y + kThumbnailTextureSize * 0.5f;

	const ImVec2 imageMin{
		centerX - txSize.x * 0.5f,
		centerY - txSize.y * 0.5f
	};

	const ImVec2 imageMax{
		centerX + txSize.x * 0.5f,
		centerY + txSize.y * 0.5f
	};

	ImGui::GetWindowDrawList()->AddImage(
		tx.textureId,
		imageMin,
		imageMax,
		tx.uv0,
		tx.uv1
	);
}

void AssetGridCell::DrawDisplayText(std::string_view text) const
{
	auto* drawList = ImGui::GetWindowDrawList();

	const float textStartY =
		min.y + kThumbnailTextureSize + 4.0f;

	const float lineHeight = ImGui::GetTextLineHeight();
	const float maxWidth = width;

	float y = textStartY;
	size_t start = 0;

	while (start < text.size())
	{
		size_t end = start;

		// Find the longest sequence that fits.
		size_t lastWhitespace = std::string_view::npos;

		while (end < text.size())
		{
			const char c = text[end];

			if (std::isspace(static_cast<unsigned char>(c)))
			{
				lastWhitespace = end;
			}

			const ImVec2 size = ImGui::CalcTextSize(
				text.data() + start,
				text.data() + end + 1
			);

			if (size.x > maxWidth)
			{
				break;
			}

			++end;
		}

		if (end == text.size())
		{
			// Everything remaining fits.
		}
		else if (lastWhitespace != std::string_view::npos &&
				 lastWhitespace > start)
		{
			end = lastWhitespace;
		}
		else if (end == start)
		{
			// A single word/character is wider than the cell.
			++end;
		}

		auto line = text.substr(start, end - start);

		// Trim trailing whitespace.
		while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
		{
			line.remove_suffix(1);
		}

		const ImVec2 lineSize = ImGui::CalcTextSize(
			line.data(),
			line.data() + line.size()
		);

		const float x =
			min.x + (width - lineSize.x) * 0.5f;

		drawList->AddText(
			ImVec2{ x, y },
			ImGui::GetColorU32(ImGuiCol_Text),
			line.data(),
			line.data() + line.size()
		);

		y += lineHeight;

		// Move past the consumed text and whitespace.
		start = end;

		while (start < text.size() &&
			std::isspace(static_cast<unsigned char>(text[start])))
		{
			++start;
		}
	}
}

auto AssetGridCell::DrawDisplayTextRenaming(std::string& renameText, 
											bool startedThisFrame) const -> RenameOutcome
{
	ImGui::SetCursorScreenPos({
		min.x,
		min.y + kThumbnailTextureSize + 4.0f
	});

	ImGui::SetNextItemWidth(width);

	if (startedThisFrame)
	{
		ImGui::SetKeyboardFocusHere();
	}

	if (ImGui::InputText("##rename", &renameText,
		ImGuiInputTextFlags_EnterReturnsTrue))
	{
		return RenameOutcome::Complete;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
		return RenameOutcome::Abort;
	}

	return RenameOutcome::Continue;
}

void AssetGridCell::DrawSelectedHighlight() const
{
	static constexpr float thickness = 2.0f;
	static constexpr float inset = thickness * 0.5f;
	
	ImGui::GetWindowDrawList()->AddRect(
		ImVec2{ min.x + inset, min.y + inset },
		ImVec2{ max.x - inset, max.y - inset },
		ImGui::GetColorU32(ImGuiCol_Header),
		2.0f,
		0,
		thickness
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

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				cell.interaction |= InteractionState::LeftClickDouble;
			}
			else
			{
				cell.interaction |= InteractionState::LeftClickSingle;
			}
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			cell.interaction |= InteractionState::RightClickSingle;
		}
	}

	return cell;
}

} // ui

#endif