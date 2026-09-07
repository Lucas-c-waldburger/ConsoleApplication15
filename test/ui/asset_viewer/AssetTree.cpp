#include "AssetTree.h"

namespace ui {

void AssetTree::Fill(const std::filesystem::path& rootPath)
{
	assets.clear();
	rootReferenceNode = AssetReferenceNode{ .id = 0, .parent = 0 };

	FillImpl(rootReferenceNode, rootPath, true);
}

void AssetTree::FillImpl(AssetReferenceNode& parentNode, const std::filesystem::path& path,
						 bool newNodeIsParent)
{
	const size_t assetIdx = assets.size();

	auto& newAsset = assets.emplace_back();
	newAsset.path = path;
	newAsset.displayName = path.filename().string();
	newAsset.type = GetAssetItemTypeFromPath(path);

	auto& newNode = (newNodeIsParent)
		? parentNode
		: parentNode.children.emplace_back();
	newNode.id = assetIdx;
	newNode.parent = parentNode.id;

	if (newAsset.type == AssetItem::Type::Directory)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			FillImpl(newNode, entry.path(), false);
		}

		// re-retrieve asset b/c vector probably reallocated
		assert(assetIdx < assets.size());
		auto& assetItem = assets[assetIdx];

		for (const auto& childNode : newNode.children)
		{
			ReverseFillItemFileChildren(childNode, assetItem);
		}
	}
}

void AssetTree::ReverseFillItemFileChildren(const AssetReferenceNode& node, AssetItem& item)
{
	assert(item.type == AssetItem::Type::Directory);

	if (node.id >= assets.size())
	{
		return;
	}

	const auto& matchingItem = assets[node.id];
	if (matchingItem.IsFile())
	{
		item.fileTypeChildren.emplace_back(node.id);
	}

	for (const auto& childNode : node.children)
	{
		ReverseFillItemFileChildren(childNode, item);
	}
}


} // ui
