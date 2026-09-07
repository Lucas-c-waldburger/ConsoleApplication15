#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../GuiTexture.h"
#include "AssetTree.h"

namespace ui {

struct AssetGridCell
{
	enum InteractionState : uint8_t
	{
		Hovered = 1 << 0,
		ClickSingle = 1 << 1,
		ClickDouble = 1 << 2,
		AnyClick = ClickSingle | ClickDouble
	};

	void DrawThumbnailTexture(const GuiTexture& tx) const;
	void DrawDisplayText(std::string_view text) const;
	void DrawSelectedHighlight() const;
	bool Clicked() const;

	static AssetGridCell Place();

	static constexpr float kThumbnailTextureSize = 64.0f;

	static constexpr float width = 80.0f;
	float height = 0.0f;
	ImVec2 min;
	ImVec2 max;
	uint8_t interaction = 0;
};

} // ui

#endif