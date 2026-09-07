#pragma once
#include "AssetTree.h"
#include "AssetViewerCommon.h"
#include "../GuiTexture.h"

#if IMGUI_ENABLED

namespace ui {

class DirectoryViewerChild
{
public:
	static Result<DirectoryViewerChild> Create(const std::filesystem::path& rootPath);

	void Draw(const AssetViewerIcons& icons, const GuiTextureConverter& converter);

	const AssetTree& GetAssetTree() const { return assetTree_; }

private:
	void DrawImpl(AssetReferenceNode& node, const AssetViewerIcons& icons, 
				  const GuiTextureConverter& converter, bool indent);

	AssetTree assetTree_;
};


} // ui

#endif

