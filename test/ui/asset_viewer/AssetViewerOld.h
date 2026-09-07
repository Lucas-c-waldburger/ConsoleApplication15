#pragma once

#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "AssetTree.h"
#include "../InspectorCommon.h"
#include "../TextureLoaderUtility.h"
#include "../../Fixtures.h"
#include <filesystem>
#include <deque>

namespace ui {

class AssetViewer
{
public:
	struct Icons 
	{
		Sprite folderClosedSmallSprite;
		Sprite folderClosedLargeSprite;
		Sprite folderOpenSmallSprite;
		Sprite folderOpenLargeSprite;
		Sprite imageFileSmallSprite;
		Sprite audioFileSmallSprite;
		Sprite fontFileSmallSprite;
		Sprite scriptFileSmallSprite;
		Sprite unknownFileSmallSprite;
	};

	GuiTexture GetNodeIconTexture(const GuiTextureConverter& converter, const AssetReferenceNode& node,
								  const AssetItem& item)
	{
		switch (item.type)
		{
		case AssetItem::Type::Directory:
			return converter.FromSprite((node.isOpen
				? icons_.folderOpenSmallSprite
				: icons_.folderClosedSmallSprite));	
		case AssetItem::Type::Image:
			return converter.FromSprite(icons_.imageFileSmallSprite);
		case AssetItem::Type::Audio:
			return converter.FromSprite(icons_.audioFileSmallSprite);
		case AssetItem::Type::Font:
			return converter.FromSprite(icons_.fontFileSmallSprite);
		case AssetItem::Type::Script:
			return converter.FromSprite(icons_.scriptFileSmallSprite);
		}
	
		return converter.FromSprite(icons_.unknownFileSmallSprite);
	}

	void DrawAssetTreeImpl(AssetReferenceNode& node, const GuiTextureConverter& converter, bool indent)
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
			selectedAssetNodeId_ = node.id;
		}

		if (ImGui::BeginDragDropSource())
		{
			const auto payloadName = GetAssetItemPayloadName(item.type);
			if (!payloadName.empty())
			{
				selectedAssetNodeId_ = node.id;

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
		auto tx = GetNodeIconTexture(converter, node, item);
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
				DrawAssetTreeImpl(ch, converter, true);
			}
		}

		if (indent)
		{
			ImGui::Unindent(ImGui::GetStyle().IndentSpacing * 0.7f);
		}

		ImGui::PopID();
	}

	inline void DrawAssetTree(const TextureRepository& auxRepo)
	{
		GuiTextureConverter converter{ auxRepo };

		DrawAssetTreeImpl(assetTree_.rootReferenceNode, converter, false);
	}

	static size_t GetPayloadAssetId(const std::string_view payloadName)
	{
		if (const auto* payload = ImGui::AcceptDragDropPayload(payloadName.data()))
		{
			return *static_cast<size_t*>(payload->Data);
		}

		return std::numeric_limits<size_t>::max();
	}

	Result<Void> HandleSpriteAssetDragDrop()
	{
		if (!ImGui::BeginDragDropTarget())
		{
			return kVoid;
		}

		if (const auto assetId = GetPayloadAssetId(kDirectoryPayloadName);
			assetId < assetTree_.assets.size())
		{
			// drag in all sprites recursively from directory
			const auto& item = assetTree_.assets[assetId];
			assert(item.type == AssetItem::Type::Directory);

			SpriteDescriptors descriptors{};
			for (const auto fileChildId : item.fileTypeChildren)
			{
				if (fileChildId >= assetTree_.assets.size())
				{
					continue;
				}

				const auto& childItem = assetTree_.assets[fileChildId];

				if (childItem.type == AssetItem::Type::Image)
				{
					descriptors.data.push_back({
						.filepath = childItem.path.string()
					});
				}
			}

			if (!descriptors.data.empty())
			{
				TRY(textureStagingArea_.GetSpriteAtlas().LoadSprites(
					SDLite::Renderer(), std::move(descriptors))
				);
			}
		}
		else if (const auto assetId = GetPayloadAssetId(kImagePayloadName);
				 assetId < assetTree_.assets.size())
		{
			const auto& item = assetTree_.assets[assetId];
			assert(item.type == AssetItem::Type::Image);

			TRY(textureStagingArea_.GetSpriteAtlas().LoadSprite(
				SDLite::Renderer(), { .filepath = item.path.string() }), sprite);
		}

		ImGui::EndDragDropTarget();

		return kVoid;
	}

	static void DrawThumbnailTexture(const GuiTexture& tx, const ImVec2 boxMin,
									 const ImVec2 boxMax)
	{
		if (tx.textureId == 0)
		{
			return;
		}

		const float thumbnailTxSize = boxMax.x - boxMin.x;

		const float scale = std::min(
			thumbnailTxSize / tx.size.x,
			thumbnailTxSize / tx.size.y
		);

		const ImVec2 size{
			tx.size.x * scale,
			tx.size.y * scale
		};

		const ImVec2 center{
			(boxMin.x + boxMax.x) * 0.5f,
			(boxMin.y + boxMax.y) * 0.5f
		};

		const ImVec2 imageMin{
			center.x - size.x * 0.5f,
			center.y - size.y * 0.5f
		};

		const ImVec2 imageMax{
			center.x + size.x * 0.5f,
			center.y + size.y * 0.5f
		};

		ImGui::GetWindowDrawList()->AddImage(
			tx.textureId,
			imageMin,
			imageMax,
			tx.uv0,
			tx.uv1
		);
	}

	enum ButtonInteractionState : uint8_t
	{
		Hovered = 1 << 0,
		ClickSingle = 1 << 1,
		ClickDouble = 1 << 2,
		AnyClick = ClickSingle | ClickDouble
	};

	static constexpr bool AnyClickOnButton(uint8_t state)
	{
		return (state & ButtonInteractionState::Hovered) && 
			   (state & ButtonInteractionState::AnyClick);
	}

	struct ThumbnailBoxData
	{
		uint8_t state = 0;
		ImVec2 boxMin;
		ImVec2 boxMax;
	};

	static ThumbnailBoxData DrawThumbnailBox(float thumbnailSize, float cellHeight)
	{
		ImGui::InvisibleButton("##thumbnail", ImVec2(thumbnailSize, cellHeight));

		ThumbnailBoxData data{
			.state = 0,
			.boxMin = ImGui::GetItemRectMin(),
			.boxMax = ImGui::GetItemRectMax()
		};

		auto* drawList = ImGui::GetWindowDrawList();

		if (ImGui::IsItemHovered())
		{
			drawList->AddRectFilled(
				data.boxMin,
				data.boxMax,
				ImGui::GetColorU32(ImGuiCol_FrameBgHovered)
			);

			data.state = ButtonInteractionState::Hovered;

			if (ImGui::IsMouseClicked(0))
			{
				if (ImGui::IsMouseDoubleClicked(0))
				{
					data.state |= ButtonInteractionState::ClickDouble;
				}
				else
				{
					data.state |= ButtonInteractionState::ClickSingle;
				}
			}
		}

		return data;
	}

	static void DrawAssetGridCellName(std::string_view name, float thumbnailSize)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnailSize);

		ImGui::TextUnformatted(name.data());

		ImGui::PopTextWrapPos();
	}

	static void DrawAssetGridCellName(std::string_view name, ImVec2 cellMin, float thumbnailSize)
	{
		const ImVec2 textPos{
			cellMin.x,
			cellMin.y + thumbnailSize + 4.0f
		};

		ImGui::GetWindowDrawList()->AddText(
			textPos,
			ImGui::GetColorU32(ImGuiCol_Text),
			name.data()
		);
	}

	static void DrawAssetGridSelectedHighlight()
	{
		ImGui::GetWindowDrawList()->AddRect(
			ImGui::GetItemRectMin(),
			ImGui::GetItemRectMax(),
			ImGui::GetColorU32(ImGuiCol_Header),
			2.0f,
			0,
			2.0f
		);
	}

	void DrawSpriteAssetGrid(const TextureRepository& auxRepo)
	{
		constexpr float thumbnailSize = 64.0f;
		constexpr float cellWidth = 80.0f;
		const float cellHeight = thumbnailSize + 4.0f + ImGui::GetTextLineHeightWithSpacing();
		const float panelWidth = ImGui::GetContentRegionAvail().x;

		const int columns = std::max(1, static_cast<int>(panelWidth / cellWidth));

		if (!ImGui::BeginTable("Sprite Asset Grid", columns))
		{
			return;
		} 

		GuiTextureConverter converter{ textureStagingArea_ };

		auto& spriteStage = textureStagingArea_.GetSpriteAtlas();

		if (!spriteAssetGridSelection_.spriteSeries.empty() &&
			spriteAssetGridSelection_.viewingInsideSeries)
		{
			auto sprites = spriteStage.GetSpriteSeries(spriteAssetGridSelection_.spriteSeries);
			if (sprites.empty())
			{
				spriteAssetGridSelection_.spriteSeries.clear();
				spriteAssetGridSelection_.sprite = {};
			}

			for (size_t i = 0; i < sprites.size(); ++i)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(static_cast<int>(i));

				auto& sprite = sprites[i];

				const auto thumbnailBoxData = DrawThumbnailBox(cellWidth, cellHeight);

				const ImVec2 boxMin = ImGui::GetItemRectMin();
				const ImVec2 boxMax = ImGui::GetItemRectMax();

				if (AnyClickOnButton(thumbnailBoxData.state))
				{
					spriteAssetGridSelection_.sprite = sprite;
				}

				auto tx = converter.FromTextureResource(sprite.resourceHandle, sprite.plot);
				assert(tx.textureId != 0);

				DrawThumbnailTexture(tx, boxMin, boxMax);

				if (spriteAssetGridSelection_.sprite == sprite)
				{
					DrawAssetGridSelectedHighlight();
				}

				auto spriteNameOp = spriteStage.GetSpriteInfo<&SpriteInfo::spriteName>(sprite);
				assert(spriteNameOp.has_value());

				DrawAssetGridCellName(*spriteNameOp, thumbnailSize);

				ImGui::PopID();
			}
		}
		else
		{
			const auto seriesNames = spriteStage.GetSpriteSeriesNames();
			if (!seriesNames.empty())
			{
				GuiTextureConverter auxConverter{ auxRepo };

				for (size_t i = 0; i < seriesNames.size(); ++i)
				{
					ImGui::TableNextColumn();
					ImGui::PushID(static_cast<int>(i));

					const auto thumbnailBoxData = DrawThumbnailBox(cellWidth, cellHeight);

					if (AnyClickOnButton(thumbnailBoxData.state))
					{
						spriteAssetGridSelection_.spriteSeries = seriesNames[i];
						spriteAssetGridSelection_.sprite = {};
					}

					auto tx = auxConverter.FromSprite(icons_.folderClosedLargeSprite);
					assert(tx.textureId != 0);

					DrawThumbnailTexture(tx, thumbnailBoxData.boxMin, thumbnailBoxData.boxMax);

					if (spriteAssetGridSelection_.spriteSeries == seriesNames[i])
					{
						DrawAssetGridSelectedHighlight();
					}

					DrawAssetGridCellName(seriesNames[i], thumbnailSize);

					ImGui::PopID();
				}
			}

			auto it = spriteStage.IterSpriteInfo<&SpriteInfo::filepath,
												 &SpriteInfo::spriteName,
												 &SpriteInfo::plot,
												 &SpriteInfo::atlasId,
												 &SpriteInfo::generation>();
			size_t counter = 0;
			for (const auto [filepath, spriteName, plot, atlasId, gen] : it)
			{
				size_t i = counter++;

				if (filepath.empty())
				{
					continue;
				}

				ImGui::TableNextColumn();
				ImGui::PushID(static_cast<int>(i));

				auto handle = Handle<TextureResource>::Create(atlasId, i, gen);

				const auto thumbnailBoxData = DrawThumbnailBox(cellWidth, cellHeight);

				if (AnyClickOnButton(thumbnailBoxData.state))
				{
					spriteAssetGridSelection_.spriteSeries.clear();
					spriteAssetGridSelection_.sprite.resourceHandle = handle;
					spriteAssetGridSelection_.sprite.plot = plot;
				}

				auto tx = converter.FromTextureResource(handle, plot);
				assert(tx.textureId != 0);

				DrawThumbnailTexture(tx, thumbnailBoxData.boxMin, thumbnailBoxData.boxMax);

				if (spriteAssetGridSelection_.sprite.resourceHandle == handle &&
					spriteAssetGridSelection_.sprite.plot == plot)
				{
					DrawAssetGridSelectedHighlight();
				}

				DrawAssetGridCellName(spriteName, thumbnailSize);

				ImGui::PopID();
			}
		}
		
		ImGui::EndTable();
	}

	void DrawSpriteView(const Sprite& sprite, const GuiTextureConverter& converter)
	{
		auto tx = converter.FromTextureResource(sprite.resourceHandle, sprite.plot);
		assert(tx.textureId != 0);

		GuiImage(tx);

		const auto& spriteStage = textureStagingArea_.GetSpriteAtlas();

		const auto info = spriteStage.GetSpriteInfo<&SpriteInfo::spriteName,
													&SpriteInfo::filepath>(sprite);
		assert(info.has_value());

		const auto& [spriteName, filepath] = *info;

		ImGui::TextUnformatted(spriteName.c_str());

		ImGui::TextUnformatted(filepath.c_str());

		const auto dimensionsStr = std::format("{} x {}", sprite.plot.rect.w, sprite.plot.rect.h);

		ImGui::TextUnformatted(dimensionsStr.c_str());
	}

	void DrawSpriteAssetViewer()
	{
		if (spriteAssetGridSelection_.NoSelection())
		{
			return;
		}

		GuiTextureConverter converter{ textureStagingArea_ };

		auto& spriteStage = textureStagingArea_.GetSpriteAtlas();

		if (spriteAssetGridSelection_.SpriteSeriesSelectedAtTopLevel())
		{
			auto sprites = spriteStage.GetSpriteSeries(spriteAssetGridSelection_.spriteSeries);

			for (size_t i = 0; i < sprites.size(); ++i)
			{
				DrawSpriteView(sprites[i], converter);

				if (i < sprites.size() - 1)
				{
					ImGui::SameLine();
				}
			}
		}
		else if (spriteStage.IsSpriteValid(spriteAssetGridSelection_.sprite))
		{
			DrawSpriteView(spriteAssetGridSelection_.sprite, converter);
		}
	}

	struct SpriteAssetGridSelection
	{
		std::string spriteSeries;
		Sprite sprite;
		bool viewingInsideSeries = false;

		bool SpriteSeriesSelectedAtTopLevel() const
		{
			return !spriteSeries.empty() && !sprite.resourceHandle.IsValid() && !viewingInsideSeries;
		}

		bool NoSelection() const
		{
			return spriteSeries.empty() && !sprite.resourceHandle.IsValid() && !viewingInsideSeries;
		}
	};

	bool Draw(const TextureRepository& auxRepo)
	{
		ImGui::SetNextWindowPos(
			ImGui::GetMainViewport()->WorkPos,
			ImGuiCond_FirstUseEver
		);

		bool isOpen = true;

		if (!ImGui::Begin("Assets", &isOpen))
		{
			ImGui::End();

			return isOpen;
		}

		const ImVec2 available = ImGui::GetContentRegionAvail();

		constexpr float bottomHeight = 200.0f;
		constexpr float leftWidth = 220.0f;
		constexpr float spacing = 4.0f;

		const float topHeight = available.y - bottomHeight - spacing;

		if (ImGui::BeginChild("AssetBrowser", ImVec2(available.x, topHeight),
							  ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
		{
			// Directories //
			ImGui::BeginChild("Directories", ImVec2(leftWidth, 0.0f), 
				(ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX));

			DrawAssetTree(auxRepo);

			ImGui::EndChild();

			ImGui::SameLine(0.0f, spacing);

			// Asset Grid //
			ImGui::BeginChild("AssetGrid", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

			if (ImGui::BeginTabBar("Tabs"))
			{
				if (ImGui::BeginTabItem("Sprites"))
				{
					DrawSpriteAssetGrid(auxRepo);

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::EndChild();

			LOG_IF_ERROR(HandleSpriteAssetDragDrop());
		}

		ImGui::EndChild();

		// Sprite Viewer //
		ImGui::BeginChild("SpriteViewer", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

		DrawSpriteAssetViewer();

		ImGui::EndChild();

		ImGui::End();

		return isOpen;
	}

	Result<Void> Init(SceneFixture& fixture)
	{
		TRY(LoadResources(fixture));

		TRY(ResourcePath::Root(), resourceRoot);
		assetTree_.Fill(resourceRoot);

		return kVoid;
	}

	const Icons& GetIcons() const noexcept { return icons_; }

private:

	Result<Void> LoadResources(SceneFixture& fixture)
	{
		auto& auxRepo = fixture.GetAuxTextureRepository();
		if (!auxRepo)
		{
			return MAKE_ERROR("Auxillary texture repository was null");
		}

		TRY(ResourcePath::Sprite("ui/editor/file_folder_open_icon_small.png"),
			folderOpenSmallPath);
		TRY(ResourcePath::Sprite("ui/editor/file_folder_open_icon_large.png"),
			folderOpenLargePath);
		TRY(ResourcePath::Sprite("ui/editor/file_folder_closed_icon_small.png"),
			folderClosedSmallPath);
		TRY(ResourcePath::Sprite("ui/editor/file_folder_closed_icon_large.png"),
			folderClosedLargePath);
		TRY(ResourcePath::Sprite("ui/editor/image_file_icon_small.png"),
			imageFileSmallPath);
		TRY(ResourcePath::Sprite("ui/editor/audio_file_icon_small.png"),
			audioFileSmallPath);
		TRY(ResourcePath::Sprite("ui/editor/font_file_icon_small.png"),
			fontFileSmallPath);
		TRY(ResourcePath::Sprite("ui/editor/script_file_icon_small.png"),
			scriptFileSmallPath);
		TRY(ResourcePath::Sprite("ui/editor/unknown_file_icon_small.png"),
			unknownFileSmallPath);

		auto& atlas = auxRepo->GetSpriteAtlas();

		TRY_ASSIGN(icons_.folderOpenSmallSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(folderOpenSmallPath) }));
		TRY_ASSIGN(icons_.folderOpenLargeSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(folderOpenLargePath) }));
		TRY_ASSIGN(icons_.folderClosedSmallSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(folderClosedSmallPath) }));
		TRY_ASSIGN(icons_.folderClosedLargeSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(folderClosedLargePath) }));
		TRY_ASSIGN(icons_.imageFileSmallSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(imageFileSmallPath) }));
		TRY_ASSIGN(icons_.audioFileSmallSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(audioFileSmallPath) }));
		TRY_ASSIGN(icons_.fontFileSmallSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(fontFileSmallPath) }));
		TRY_ASSIGN(icons_.unknownFileSmallSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(unknownFileSmallPath) }));
		TRY_ASSIGN(icons_.scriptFileSmallSprite, atlas.LoadSprite(
			fixture.GetRenderer(), { .filepath = std::move(scriptFileSmallPath) }));

		return kVoid;
	}

	size_t selectedAssetNodeId_ = std::numeric_limits<size_t>::max();
	SpriteAssetGridSelection spriteAssetGridSelection_;
	AssetTree assetTree_;
	Icons icons_;
	AssetItem::Type activeAssetGridType = AssetItem::Type::Unknown;
	TextureRepository textureStagingArea_;
};













} // ui

#endif