#pragma once

#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "InspectorCommon.h"
#include "TextureLoaderUtility.h"
#include <filesystem>
#include <deque>

namespace ui {

//class AssetWindow
//{
//public:
//	static inline const std::unordered_set<std::string_view> kImageExtensions = {
//		".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".webp", ".svg"
//	};
//
//	static inline const std::unordered_set<std::string_view> kAudioExtensions = {
//		".mp3", ".ogg", ".flac", ".wav"
//	};
//
//	static inline const std::unordered_set<std::string_view> kFontExtensions = {
//		".ttf"
//	};
//
//	struct Config
//	{
//		std::string spritePath;
//		std::string fontPath;
//		std::string audioPath;
//	};
//
//
//	std::string selectedSpriteGrouping_;
//
//	struct Icons 
//	{
//		Sprite folderClosedSmallSprite;
//		Sprite folderClosedLargeSprite;
//		Sprite folderOpenSmallSprite;
//		Sprite imageSmallSprite;
//	};
//
//	Icons icons_;
//
//	struct TextureDisplayInfo
//	{
//		std::string name;
//		std::string dimensions;
//		GuiTexture texture;
//	};
//
//	struct FolderNode
//	{
//		enum Type
//		{
//			Unknown,
//			Directory,
//			Image,
//			Font,
//			Audio
//		};
//
//		std::filesystem::path path;
//		std::string displayName;
//		Type type = Type::Unknown;
//		Handle<TextureResource> resourceHandle;
//		bool isOpen = false;
//		std::vector<FolderNode> children;
//	};
//
//	struct FolderNodes
//	{
//		FolderNode spriteFolder;
//		FolderNode fontFolder;
//		FolderNode audioFolder;
//	};
//
//	FolderNode* selectedFolderNode_ = nullptr;
//
//	std::unordered_map<std::filesystem::path, std::vector<TextureDisplayInfo>> 
//	folderNodePathToTextureDisplayInfo_;
//
//	static std::string MakeSpriteDimensionsString(const AtlasPlot& plot)
//	{
//		return std::format("{} x {}", plot.rect.w, plot.rect.h);
//	}
//
//	static TextureDisplayInfo SpriteToTextureDisplayInfo(const Sprite& sprite, 
//														 std::string&& displayName,
//														 const GuiTextureConverter& converter)
//	{
//		return {
//			std::move(displayName),
//			MakeSpriteDimensionsString(sprite.plot),
//			converter.FromSprite(sprite)
//		};
//	}
//
//	std::vector<TextureDisplayInfo> GetSpriteTextureDisplayInfo(TextureRepository& repo)
//	{
//		GuiTextureConverter converter{ repo };
//		std::vector<TextureDisplayInfo> textures;
//
//		const auto& spriteAtlas = repo.GetSpriteAtlas();
//
//		if (!selectedSpriteGrouping_.empty()) // specific series
//		{
//			auto sprites = spriteAtlas.GetSpriteSeries(selectedSpriteGrouping_);
//			textures.reserve(sprites.size());
//
//			for (const auto& sprite : sprites)
//			{
//				const auto& spritePath = spriteAtlas.GetSpriteInfo<&SpriteInfo::filepath>(sprite);
//
//				auto name = spritePath.has_value()
//					? std::filesystem::path(*spritePath).stem().string()
//					: "";
//
//				textures.emplace_back(
//					std::move(name),
//					MakeSpriteDimensionsString(sprite.plot),
//					sprite.resourceHandle,
//					converter.FromSprite(sprite)
//				);
//			}
//		}
//		else  // top level
//		{
//			textures.reserve(spriteAtlas.GetSpriteCount());
//
//			std::string_view currentSeriesName;
//			auto folderTx = converter.FromSprite(icons_.folderClosedLargeSprite);
//
//			auto iter = spriteAtlas.IterSpriteInfo<&SpriteInfo::atlasId, &SpriteInfo::plot,
//												   &SpriteInfo::filepath, &SpriteInfo::seriesName>();
//			size_t spriteIdx = 0;
//
//			for (const auto& [atlasId, plot, filepath, seriesName] : iter)
//			{
//				if (!seriesName.empty())
//				{
//					if (currentSeriesName != seriesName)
//					{
//						currentSeriesName = seriesName;
//
//						textures.emplace_back(seriesName, Handle<TextureResource>{}, folderTx);
//					}
//				}
//				else
//				{
//					const auto handle = Handle<TextureResource>::Create(atlasId, spriteIdx);
//
//					textures.emplace_back(
//						std::filesystem::path(filepath).stem().string(),
//						MakeSpriteDimensionsString(plot),
//						handle,
//						converter.FromTextureResource(handle, plot)
//					);
//				}
//
//				++spriteIdx;
//			}
//		}
//
//		return textures;
//	}
//
//	void DrawSpritePreview(const TextureDisplayInfo& displayInfo, const TextureRepository& repo)
//	{
//		if (ImGui::BeginChild("Preview"))
//		{
//			float previewZoom = 1.0f;
//
//			ImGui::SliderFloat("Zoom", &previewZoom, 0.1f, 16.0f);
//
//			auto tx = displayInfo.texture;
//			tx.size.x *= previewZoom;
//			tx.size.y *= previewZoom;
//
//			GuiImage(tx);
//
//			ImGui::TextUnformatted(displayInfo.name.c_str());
//
//			ImGui::SameLine();
//
//			ImGui::TextUnformatted(displayInfo.dimensions.c_str());
//		}
//
//		ImGui::EndChild();
//	}
//
//	Result<FolderNodes> MakeFolderNodes(const Config& config, const TextureRepository& repo)
//	{
//		static constexpr auto makeNode = [](const auto& path, const auto& validExts, auto type,
//											const auto& converter) 
//		-> Result<FolderNode> {
//			if (!std::filesystem::exists(path))
//			{
//				return MAKE_ERROR_FMT("Asset path '{}' does not exist", path);
//			}
//			if (!std::filesystem::is_directory(path))
//			{
//				return MAKE_ERROR_FMT("Asset path '{}' is not a directory", path);
//			}
//
//			folderNodePathToTextureDisplayInfo_.try_emplace(path, std::vector<TextureDisplayInfo>{});
//
//			return MakeFolderNodeImpl(path, validExts, type, converter);
//		};
//
//		FolderNodes nodes{};
//		GuiTextureConverter converter{ repo };
//
//		TRY_ASSIGN(nodes.spriteFolder, 
//			makeNode(config.spritePath, kImageExtensions, FolderNode::Type::Image, converter));
//
//		return nodes;
//	}
//
//	FolderNode MakeFolderNodeImpl(const std::filesystem::path& path, 
//								  const std::unordered_set<std::string>& validExtensions,
//								  FolderNode::Type fileType, const GuiTextureConverter& converter)
//	{
//		FolderNode node{
//			.path = path,
//			.displayName = path.filename().string()
//		};
//
//		if (std::filesystem::is_directory(path))
//		{
//			node.type = FolderNode::Type::Directory;
//			auto& textureDisplayInfoVec = folderNodePathToTextureDisplayInfo_[path];
//
//			for (const auto& entry : std::filesystem::directory_iterator(path))
//			{
//				auto& child = node.children.emplace_back(
//					MakeFolderNodeImpl(entry.path(), validExtensions, fileType, converter)
//				);
//
//				if (child.type == FolderNode::Type::Unknown)
//				{
//					node.children.pop_back();
//				}
//				else if (child.type == FolderNode::Type::Directory)
//				{
//					textureDisplayInfoVec.emplace_back(
//						child.displayName,
//						"",
//						Handle<TextureResource>{},
//						converter.FromSprite(icons_.folderClosedLargeSprite)
//					);
//				}
//			}
//		}
//		else if (std::filesystem::is_regular_file(path))
//		{
//			auto ext = path.extension().string();
//		
//			if (validExtensions.contains(ext))
//			{
//				node.type = fileType;
//			}
//		}
//
//		return node;
//	}
//
//	GuiTexture GetNodeIconTexture(const GuiTextureConverter& converter, const FolderNode& node)
//	{
//		switch (node.type)
//		{
//		case FolderNode::Directory:
//			return converter.FromSprite((node.isOpen
//				? icons_.folderOpenSmallSprite
//				: icons_.folderClosedSmallSprite));
//
//		case FolderNode::Image:
//			return converter.FromSprite(icons_.imageSmallSprite);
//		}
//
//		return {};
//	}
//
//	void DrawFolderNodesImpl(FolderNode& node, const GuiTextureConverter& converter)
//	{
//		ImGui::PushID(&node);
//
//		ImGui::Indent();
//
//		const float rowHeight = ImGui::GetFrameHeight();
//		const bool isLeaf = node.children.empty();
//		const bool selected = selectedFolderNode_ == &node;
//
//		if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_SpanAllColumns,
//			ImVec2(0, rowHeight)))
//		{
//			selectedFolderNode_ = &node;
//		}
//
//		if (node.type == FolderNode::Type::Directory)
//		{
//			ImGui::BeginDisabled(isLeaf);
//
//			if (ImGui::ArrowButton("##arrow", (node.isOpen && !isLeaf) ? ImGuiDir_Down : ImGuiDir_Right))
//			{
//				node.isOpen = !node.isOpen;
//			}
//
//			ImGui::EndDisabled();
//		}
//
//		if (ImGui::BeginDragDropSource())
//		{
//			auto pathStr = node.path.generic_string();
//
//			ImGui::SetDragDropPayload("ASSET_IMAGE", 
//				pathStr.data(),
//				pathStr.size() + 1
//			);
//
//			ImGui::TextUnformatted(node.displayName.c_str());
//
//			ImGui::EndDragDropSource();
//		}
//
//		ImGui::SameLine();
//
//		auto tx = GetNodeIconTexture(converter, node);
//		assert(tx.textureId != 0);
//		tx.size.x = rowHeight;
//		tx.size.y = rowHeight;
//
//		GuiImage(tx);
//
//		ImGui::SameLine();
//
//		ImGui::TextUnformatted(node.displayName.c_str());
//
//		if (!isLeaf && node.isOpen)
//		{
//			for (auto& ch : node.children)
//			{
//				DrawFolderNodesImpl(ch, converter);
//			}
//		}
//
//		ImGui::Unindent();
//
//		ImGui::PopID();
//	}
//
//	void DrawAssetGridTable(const TextureRepository& repo)
//	{
//		constexpr float thumbnailSize = 64.0f;
//		constexpr float padding = 8.0f;
//
//		const float panelWidth = ImGui::GetContentRegionAvail().x;
//		const int columns = std::max(1, static_cast<int>(panelWidth / (thumbnailSize + padding)));
//
//		if (!ImGui::BeginTable("Asset Grid", columns))
//		{
//			return;
//		}
//
//		if (!selectedFolderNode_)
//		{
//			ImGui::EndTable();
//			return;
//		}
//
//		GuiTextureConverter converter{ repo };
//
//		static constexpr auto drawImgButton = [](const TextureDisplayInfo& displayInfo, 
//												 const GuiTextureConverter& converter)
//		{
//			auto tx = displayInfo.texture;
//			tx.size.x = thumbnailSize;
//			tx.size.y = thumbnailSize;
//
//			const bool pressed = GuiImageButton(displayInfo.name, tx);
//
//			ImGui::TextWrapped("%s", displayInfo.name.c_str());
//
//			return pressed;
//		};
//
//		if (selectedFolderNode_->type == FolderNode::Type::Directory)
//		{
//			auto it = folderNodePathToTextureDisplayInfo_.find(selectedFolderNode_->path);	
//			if (it == folderNodePathToTextureDisplayInfo_.end())
//			{
//				return;
//			}
//
//			for (const auto& textureDisplay : it->second)
//			{
//				const bool pressed = drawImgButton(textureDisplay, converter);
//			}
//		}
//		else if (selectedFolderNode_->type == FolderNode::Type::Image)
//		{
//			const bool pressed = 
//		}
//
//		ImGui::EndTable();
//	}
//
//	void DrawAssetGrid(const GuiTextureConverter& converter)
//	{
//		if (ImGui::BeginChild("AssetGrid", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders))
//		{
//			if (!selectedFold erNode_)
//			{
//				return;
//			}
//
//			if (selectedFolderNode_->type == FolderNode::Type::Directory)
//			{
//				for (const auto& ch : selectedFolderNode_->children)
//				{
//					if (ch.type == )
//				}
//			}
//
//
//			ImGui::EndChild();
//		}
//	}
//
//	Result<Void> LoadSpriteFromPath(SDL_Renderer* r, TextureRepository& repo, 
//								    const std::filesystem::path& spritePath)
//	{
//		auto ext = spritePath.extension().string();
//		if (!kImageExtensions.contains(ext))
//		{
//			return MAKE_ERROR_FMT("File '{}' is not a supported image type", spritePath);
//		}
//
//		SpriteDescriptor descriptor{ .filepath = spritePath.string() };
//
//		TRY(repo.GetSpriteAtlas().LoadSprite(r, std::move(descriptor)), sprite);
//
//		folderNodePathToTextureDisplayInfo_[config_.spritePath].emplace_back(
//			SpriteToTextureDisplayInfo(
//				sprite,
//				spritePath.stem().string(),
//				GuiTextureConverter{ repo }
//		));
//
//		return kVoid;
//	}
//
//	Result<Void> LoadSpritesFromFolder(SDL_Renderer* r, TextureRepository& repo, 
//									   const std::filesystem::path& folderPath)
//	{
//		SpriteDescriptors descriptors{
//			.seriesName = folderPath.filename().string()
//		};
//		std::vector<std::string> displayNames;
//
//		auto iter = std::filesystem::directory_iterator(folderPath);
//		const size_t spriteCount = std::distance(iter, std::filesystem::directory_iterator{});
//
//		descriptors.data.reserve(spriteCount);
//		displayNames.reserve(spriteCount);
//
//		for (const auto& entry : std::filesystem::directory_iterator(folderPath))
//		{
//			if (std::filesystem::is_regular_file(entry))
//			{
//				auto ext = entry.path().extension().string();
//				if (kImageExtensions.contains(ext))
//				{
//					descriptors.data.emplace_back(SpriteDescriptor{
//						.filepath = entry.path().string(),
//					});
//					displayNames.emplace_back(entry.path().stem().string());
//				}
//			}
//		}
//
//		TRY(repo.GetSpriteAtlas().LoadSprites(r, std::move(descriptors)), sprites);
//
//		auto& textures = folderNodePathToTextureDisplayInfo_[folderPath];
//		textures.reserve(sprites.size());
//
//		for (size_t i = 0; i < sprites.size(); ++i)
//		{
//			textures.emplace_back(SpriteToTextureDisplayInfo(
//				sprites[i], std::move(displayNames[i]), GuiTextureConverter{ repo })
//			);
//		}
//
//		return kVoid;
//	}
//
//	Result<Void> HandleAssetGridDragDrop(SDL_Renderer* r, TextureRepository& repo)
//	{
//		if (!ImGui::BeginDragDropTarget())
//		{
//			return kVoid;
//		}
//
//		if (const auto* payload = ImGui::AcceptDragDropPayload("ASSET_IMAGE"))
//		{
//			GuiTextureConverter converter{ repo };
//
//			std::filesystem::path path{ static_cast<const char*>(payload->Data) };
//
//			if (folderNodePathToTextureDisplayInfo_.contains(path))
//			{
//				return kVoid;
//			}
//			
//			if (std::filesystem::is_directory(path))
//			{
//				TRY(LoadSpritesFromFolder(r, repo, path));
//			}
//			else if (std::filesystem::is_regular_file(path))
//			{
//				TRY(LoadSpriteFromPath(r, repo, path));
//			}
//		}
//
//		ImGui::EndDragDropTarget();
//
//		return kVoid;
//	}
//
//	// - sprite folder comes in
//	//	- if series name specified, gets grouped under folder of series name
//	//	 - make descriptors with series name
//	//	- else gets added individually under "sprites" folder
//	//   - make descriptors no series name
//	// - spriteAtlas.LoadSprites(descriptors)
//	// - inside preview list panel...
//	//  - look at what folder is looked at in other panel (top level sprites or series folder)
//	//	- if top level sprites folder, iterate through SpriteInfo and draw all icons
//	//  - else if sprite series folder, just draw sprites for that series
//
//	//void DrawFolderRow(const std::filesystem::directory_entry& entry, size_t indentFactor)
//	//{
//	//	const bool isDir = entry.is_directory()
//	//} 
//
//private:
//	//void GatherSpriteDescriptors(SpriteDescriptorPackage& package, const std::filesystem::path& curPath)
//	//{
//	//	if (std::filesystem::is_directory(curPath))
//	//	{
//	//		for (const auto& entry : std::filesystem::directory_iterator(curPath))
//	//		{
//	//			GatherSpriteDescriptors(package, entry.path());
//	//		}
//	//	}
//	//	else if (std::filesystem::is_regular_file(curPath))
//	//	{
//	//		auto ext = curPath.extension().string();
//
//	//		if (kImageExtensions.contains(ext))
//	//		{
//	//			TRY(repo.GetSpriteAtlas)
//	//		}
//	//	}
//	//}
//	//Result<Void> LoadSprites(TextureRepository& repo, const std::filesystem::path& curPath)
//	//{
//	//	if (std::filesystem::is_directory(curPath))
//	//	{
//	//		for (const auto& entry : std::filesystem::directory_iterator(curPath))
//	//		{
//	//			TRY(LoadSprites(repo, entry.path()));
//	//		}
//	//	}
//	//	else if (std::filesystem::is_regular_file(curPath))
//	//	{
//	//		auto ext = curPath.extension().string();
//
//	//		if (kImageExtensions.contains(ext))
//	//		{
//	//			TRY(repo.GetSpriteAtlas)
//	//		}
//	//	}
//	//}
//	//Result<Void> InitFolder(std::string_view assetTypePathStr)
//	//{
//	//	auto assetPath = std::filesystem::path(assetTypePathStr);
//	//	if (!std::filesystem::exists(assetPath))
//	//	{
//	//		return MAKE_ERROR_FMT("Asset path '{}' does not exist", assetTypePathStr);
//	//	}
//	//	if (!std::filesystem::is_directory(assetPath))
//	//	{
//	//		return MAKE_ERROR_FMT("Asset path '{}' is not a directory", assetTypePathStr);
//	//	}
//
//
//	//}
//
//	/*void PopulateFoldersImpl(const std::filesystem::path& curPath, Folder& curFolder)
//	{
//		if (std::filesystem::is_directory(curPath))
//		{
//			curFolder.flags |= Folder::IsDirectory;
//
//			for (const auto& entry : std::filesystem::directory_iterator(curPath))
//			{
//				auto& child = curFolder.children.emplace_back();
//
//				PopulateFoldersImpl(entry.path(), child);
//			}
//		}
//		else if (std::filesystem::is_regular_file(curPath))
//		{
//			auto ext = curPath.extension().string();
//
//			if (kImageExtensions.contains(ext))
//			{
//				curFolder.flags |= Folder::IsImage;
//			}
//			else if (kAudioExtensions.contains(ext))
//			{
//				curFolder.flags |= Folder::IsAudio;
//			}
//			else if (kFontExtensions.contains(ext))
//			{
//				curFolder.flags |= Folder::IsFont;
//			}
//
//			curFolder.displayName = curPath.stem().string();
//		}
//	}*/
//
//
//	//Folder spriteFolder{ .displayName = "sprites" };
//	//Folder fontFolder{ .displayName = "fonts" };
//	//Folder audioFolder{ .displayName = "audio" };
//	Config config_;
//};

class AssetWindow
{
public:
	static inline const std::unordered_set<std::string_view> kImageExtensions = {
		".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".webp", ".svg"
	};
	
	static inline const std::unordered_set<std::string_view> kAudioExtensions = {
		".mp3", ".ogg", ".flac", ".wav"
	};
	
	static inline const std::unordered_set<std::string_view> kFontExtensions = {
		".ttf"
	};

	static inline constexpr std::string_view kDirectoryPayloadName = "ASSET_DIRECTORY";
	static inline constexpr std::string_view kImagePayloadName = "ASSET_IMAGE";
	static inline constexpr std::string_view kFontPayloadName = "ASSET_FONT";
	static inline constexpr std::string_view kAudioPayloadName = "ASSET_AUDIO";

	struct AssetItem
	{
		enum Type
		{
			Unknown,
			Directory,
			Image,
			Font,
			Audio
		};

		std::filesystem::path path;
		std::string displayName;
		Type type = Type::Unknown;
	};

	struct AssetReferenceNode
	{
		size_t id = 0;
		size_t parent = 0;
		bool isOpen = false;
		std::vector<AssetReferenceNode> children;
	};

	static AssetItem::Type GetAssetItemType(const std::filesystem::path& path)
	{
		auto ext = path.extension().string();

		if (std::filesystem::is_directory(path))
		{
			return AssetItem::Type::Directory;
		}
		else if (std::filesystem::is_regular_file(path) && path.has_extension())
		{
			const auto ext = path.extension().string();

			if (kImageExtensions.contains(ext))
			{
				return AssetItem::Type::Image;
			}
			if (kFontExtensions.contains(ext))
			{
				return AssetItem::Type::Font;
			}
			if (kAudioExtensions.contains(ext))
			{
				return AssetItem::Type::Audio;
			}
		}
		
		return AssetItem::Type::Unknown;
	}

	std::string_view GetAssetItemPayloadName(const AssetItem& item)
	{
		switch (item.type)
		{
		case AssetItem::Type::Directory: return kDirectoryPayloadName;
		case AssetItem::Type::Image: return kImagePayloadName;
		case AssetItem::Type::Font: return kFontPayloadName;
		case AssetItem::Type::Audio: return kAudioPayloadName;
		}

		return {};
	}

	struct AssetTree
	{
		std::vector<AssetItem> assets;
		AssetReferenceNode rootReferenceNode;

		void Fill(const std::filesystem::path& rootPath)
		{
			assets.clear();
			rootReferenceNode = AssetReferenceNode{ .id = 0, .parent = 0 };

			FillImpl(rootReferenceNode, rootPath);
		}

		void FillImpl(AssetReferenceNode& parentNode, const std::filesystem::path& path)
		{
			const size_t assetIdx = assets.size();

			auto& newAsset = assets.emplace_back();
			newAsset.path = path;
			newAsset.displayName = path.filename().string();
			newAsset.type = GetAssetItemType(path);

			auto& newNode = parentNode.children.emplace_back();
			newNode.id = assetIdx;
			newNode.parent = parentNode.id;

			if (newAsset.type == AssetItem::Type::Directory)
			{
				for (const auto& entry : std::filesystem::directory_iterator(path))
				{
					FillImpl(newNode, entry.path());
				}
			}
		}
	};

	//struct GridNavigationStack
	//{
	//	std::vector<Sprite> sprites;
	//	AssetReferenceNode rootReferenceNode;
	//	std::deque<size_t> navStack{ 0 };

	//	void Fill(const TextureRepository& repo)
	//};

	struct Icons 
	{
		Sprite folderClosedSmallSprite;
		Sprite folderClosedLargeSprite;
		Sprite folderOpenSmallSprite;
		Sprite folderOpenLargeSprite;
		Sprite imageSmallSprite;
	};

	GuiTexture GetNodeIconTexture(const GuiTextureConverter& converter, const AssetReferenceNode& node,
								  const AssetItem& item)
	{
		switch (item.type)
		{
		case AssetItem::Directory:
			return converter.FromSprite((node.isOpen
				? icons_.folderOpenSmallSprite
				: icons_.folderClosedSmallSprite));
	
		case AssetItem::Image:
			return converter.FromSprite(icons_.imageSmallSprite);
		}
	
		return {};
	}

	inline void DrawAssetTreeImpl(AssetReferenceNode& node, AssetTree& tree,
								  const GuiTextureConverter& converter)
	{
		assert(node.id < tree.assets.size());
		auto& item = tree.assets[node.id];

		ImGui::PushID(node.id);

		ImGui::Indent();

		const float rowHeight = ImGui::GetFrameHeight();
		const bool isLeaf = node.children.empty();
		assert(isLeaf == AssetItem::Type::Directory);

		const bool selected = selectedAssetNodeId_ == node.id;

		if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_SpanAllColumns,
			ImVec2(0, rowHeight)))
		{
			selectedAssetNodeId_ = node.id;
		}

		if (item.type == AssetItem::Type::Directory)
		{
			ImGui::BeginDisabled(isLeaf);

			if (ImGui::ArrowButton("##arrow", (node.isOpen && !isLeaf) ? ImGuiDir_Down : ImGuiDir_Right))
			{
				node.isOpen = !node.isOpen;
			}

			ImGui::EndDisabled();
		}

		if (ImGui::BeginDragDropSource())
		{
			const auto payloadName = GetAssetItemPayloadName(item);
			if (!payloadName.empty())
			{
				ImGui::SetDragDropPayload(payloadName.data(), &node.id, sizeof(node.id));
			}

			ImGui::TextUnformatted(item.displayName.c_str());

			ImGui::EndDragDropSource();
		}

		ImGui::SameLine();

		auto tx = GetNodeIconTexture(converter, node, item);
		assert(tx.textureId != 0);
		tx.size.x = rowHeight;
		tx.size.y = rowHeight;

		GuiImage(tx);

		ImGui::SameLine();

		ImGui::TextUnformatted(item.displayName.c_str());

		if (!isLeaf && node.isOpen)
		{
			for (auto& ch : node.children)
			{
				DrawAssetTreeImpl(ch, tree, converter);
			}
		}

		ImGui::Unindent();

		ImGui::PopID();
	}

	template <typename T>
	struct GridCell
	{
		Sprite displaySprite;
		std::string displayName;
		T data;
	};

	template <typename T>
	struct AssetGrid
	{
		std::vector<GridCell<T>> cells;
		size_t selectedCell = std::numeric_limits<size_t>::max();
	};

	using SpriteGridCell = GridCell<Sprite>;
	using SpriteAssetGrid = AssetGrid<Sprite>;

	Result<Void> HandleSpriteAssetDragDrop(const AssetItem& item, TextureRepository& repo)
	{
		if (item.type != AssetItem::Type::Image)
		{
			return kVoid;
		}

		TRY(repo.GetSpriteAtlas().LoadSprite(SDLite::Renderer(), {.filepath = item.path.string()}), 
			sprite);

		spriteAssetGrid_.cells.emplace_back(
			sprite,
			item.displayName,
			sprite
		);

		return kVoid;
	}

	Result<Void> HandleAssetGridDragDrop(TextureRepository& repo)
	{
		if (!ImGui::BeginDragDropTarget())
		{
			return kVoid;
		}

		if (const auto* payload = ImGui::AcceptDragDropPayload(kImagePayloadName.data()))
		{
			const auto assetId = *static_cast<size_t*>(payload->Data);

			if (assetId < assetTree_.assets.size())
			{
				const auto& item = assetTree_.assets[assetId];

				LOG_IF_ERROR(HandleSpriteAssetDragDrop(item, repo));
			}
		}

		ImGui::EndDragDropTarget();

		return kVoid;
	}

	void DrawSpriteAssetGridTable(const TextureRepository& repo)
	{
		constexpr float thumbnailSize = 64.0f;
		constexpr float padding = 8.0f;

		const float panelWidth = ImGui::GetContentRegionAvail().x;
		const int columns = std::max(1, static_cast<int>(panelWidth / (thumbnailSize + padding)));

		if (!ImGui::BeginTable("Sprite Asset Grid", columns))
		{
			return;
		} 

		GuiTextureConverter converter{ repo };

		auto& spriteAtlas = repo.GetSpriteAtlas();

		if (!selectedSpriteSeries_.empty())
		{
			auto sprites = spriteAtlas.GetSpriteSeries(selectedSpriteSeries_);

			for (size_t i = 0; i < sprites.size(); ++i)
			{
				ImGui::NextColumn();
				ImGui::PushID(static_cast<int>(i));

				auto& sprite = sprites[i];

				auto tx = converter.FromTextureResource(sprite.resourceHandle, sprite.plot);
			}
		}


		auto it = spriteAtlas.IterSpriteInfo<&SpriteInfo::filepath, 
											 &SpriteInfo::spriteName, 
											 &SpriteInfo::plot>();

		for (const auto [filepath, spriteName, plot] : it)
		{

		}

		//static constexpr auto drawImgButton = [](std::string_view displayName, 
		//										 const GuiTextureConverter& converter)
		//{
		//	auto tx = displayInfo.texture;
		//	tx.size.x = thumbnailSize;
		//	tx.size.y = thumbnailSize;

		//	const bool pressed = GuiImageButton(displayInfo.name, tx);

		//	ImGui::TextWrapped("%s", displayInfo.name.c_str());

		//	return pressed;
		//};

		//if (selectedFolderNode_->type == FolderNode::Type::Directory)
		//{
		//	auto it = folderNodePathToTextureDisplayInfo_.find(selectedFolderNode_->path);	
		//	if (it == folderNodePathToTextureDisplayInfo_.end())
		//	{
		//		return;
		//	}

		//	for (const auto& textureDisplay : it->second)
		//	{
		//		const bool pressed = drawImgButton(textureDisplay, converter);
		//	}
		//}
		//else if (selectedFolderNode_->type == FolderNode::Type::Image)
		//{
		//	const bool pressed = 
		//}

		ImGui::EndTable();
	}

private:
	size_t selectedAssetNodeId_ = std::numeric_limits<size_t>::max();
	std::string selectedSpriteSeries_;
	SpriteAssetGrid spriteAssetGrid_;
	AssetTree assetTree_;
	Icons icons_;
};













} // ui

#endif