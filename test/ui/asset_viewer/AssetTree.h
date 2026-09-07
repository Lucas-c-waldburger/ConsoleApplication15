#pragma once
#include <cassert>
#include "AssetItem.h"

namespace ui {

struct AssetReferenceNode
{
	size_t id = 0;
	size_t parent = 0;
	bool isOpen = false;
	std::vector<AssetReferenceNode> children;
};

class AssetTree
{
public:
	std::vector<AssetItem> assets;
	AssetReferenceNode rootReferenceNode;

	void Fill(const std::filesystem::path& rootPath);

private:
	void FillImpl(AssetReferenceNode& parentNode, const std::filesystem::path& path,
				  bool newNodeIsParent);

	void ReverseFillItemFileChildren(const AssetReferenceNode& node, AssetItem& item);
};


} // ui