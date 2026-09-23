#include "AssetViewerUtil.h"

#if IMGUI_ENABLED
#include <imgui_internal.h>

namespace ui {

void DrawCheckerboard(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, 
					  const ImVec2& size_arg, float grid_step)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
	{
		return;
	}

	ImGuiContext& g = *GImGui;
	const ImGuiID id = window->GetID(desc_id);

	const float default_size = ImGui::GetFrameHeight();
	const ImVec2 size(size_arg.x == 0.0f
		? default_size
		: size_arg.x,
		size_arg.y == 0.0f
		? default_size
		: size_arg.y);

	const ImRect bb(window->DC.CursorPos,
		ImVec2(window->DC.CursorPos.x + size.x,
			window->DC.CursorPos.y + size.y));

	ImGui::ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
	if (!ImGui::ItemAdd(bb, id))
	{
		return;
	}

	if (flags & (ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaOpaque))
	{
		flags &= ~(ImGuiColorEditFlags_AlphaNoBg | ImGuiColorEditFlags_AlphaPreviewHalf);
	}

	ImVec4 col_rgb = col;
	if (flags & ImGuiColorEditFlags_InputHSV)
	{
		ImGui::ColorConvertHSVtoRGB(col_rgb.x, col_rgb.y, col_rgb.z, col_rgb.x, col_rgb.y, col_rgb.z);
	}

	ImVec4 col_rgb_without_alpha(col_rgb.x, col_rgb.y, col_rgb.z, 1.0f);
	
	float rounding = ImMin(g.Style.FrameRounding, grid_step * 0.5f);
	ImRect bb_inner = bb;
	float off = 0.0f;

	if ((flags & ImGuiColorEditFlags_NoBorder) == 0)
	{
		off = -0.75f;
		bb_inner.Expand(off);
	}
	if ((flags & ImGuiColorEditFlags_AlphaPreviewHalf) && col_rgb.w < 1.0f)
	{
		float mid_x = IM_ROUND((bb_inner.Min.x + bb_inner.Max.x) * 0.5f);

		if ((flags & ImGuiColorEditFlags_AlphaNoBg) == 0)
		{
			ImGui::RenderColorRectWithAlphaCheckerboard(
				window->DrawList,
				ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y),
				bb_inner.Max,
				ImGui::GetColorU32(col_rgb),
				col_rgb.w,
				grid_step,
				ImVec2(-grid_step + off, off),
				rounding,
				ImDrawFlags_RoundCornersRight);
		}
		else
		{
			window->DrawList->AddRectFilled(
				ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y),
				bb_inner.Max,
				ImGui::GetColorU32(col_rgb),
				rounding,
				ImDrawFlags_RoundCornersRight);

			window->DrawList->AddRectFilled(
				bb_inner.Min,
				ImVec2(mid_x, bb_inner.Max.y),
				ImGui::GetColorU32(col_rgb_without_alpha),
				rounding,
				ImDrawFlags_RoundCornersLeft);
		}
	}
	else
	{
		ImVec4 col_source = (flags & ImGuiColorEditFlags_AlphaOpaque)
			? col_rgb_without_alpha
			: col_rgb;

		if (col_source.w < 1.0f && (flags & ImGuiColorEditFlags_AlphaNoBg) == 0)
		{
			ImGui::RenderColorRectWithAlphaCheckerboard(
				window->DrawList,
				bb_inner.Min,
				bb_inner.Max,
				ImGui::GetColorU32(col_source),
				col_source.w,
				grid_step,
				ImVec2(off, off),
				rounding);
		}
		else
		{
			window->DrawList->AddRectFilled(
				bb_inner.Min,
				bb_inner.Max,
				ImGui::GetColorU32(col_source),
				rounding);
		}
	}
	ImGui::RenderNavCursor(bb, id);
	if ((flags & ImGuiColorEditFlags_NoBorder) == 0)
	{
		if (g.Style.FrameBorderSize > 0.0f)
		{
			ImGui::RenderFrameBorder(bb.Min, bb.Max, rounding);
		}
		else
		{
			window->DrawList->AddRect(
				bb.Min, 
				bb.Max,
				ImGui::GetColorU32(ImGuiCol_FrameBg), 
				rounding);
		}
	}
}



} // ui

#endif