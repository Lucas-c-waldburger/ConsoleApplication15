#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED 

#include "FileTreeView.h"
#include "../../atlas/NewTextureRepository.h"

namespace ui {

class SpritePicker
{
public:
    struct ToolTipImage
    {
        ImTextureID textureId = 0;
        ImVec2 size;
        ImVec2 uv0;
        ImVec2 uv1;

        void Draw() const
        {
            if (textureId == 0)
            {
                return;
            }

            ImGui::BeginTooltip();

            ImGui::Image(textureId, size, uv0, uv1);

            ImGui::EndTooltip();
        }
    };

    static const fs::path& GetSpriteDirectory()
    {
        static const auto dirPath = FilePathUtility::GetRootPath()
            / kResourcesDirName
            / kSpritesDirName;

        assert(fs::exists(dirPath));

        return dirPath;
    }

	struct FileTreeState
	{
        FileTreeState()
        {
            const auto& spriteDir = GetSpriteDirectory();

            topLevelPath = spriteDir;
            currentPath = spriteDir;
        }

        fs::path topLevelPath;
        fs::path currentPath;
		std::optional<fs::path> selectedFile;
	};

    struct SpriteData
    {
        Sprite sprite;
        std::string spriteName;
    };

	struct SpriteState
	{
		std::optional<SpriteData> hoveredSprite;
		std::optional<SpriteData> selectedSprite;
        std::optional<ToolTipImage> toolTipImage;
	};

	SpritePicker() = default;

	bool Draw(TextureRepository& textureRepo)
	{
        assert(fs::exists(fileTreeState_.currentPath));

        isOpen_ = true;
        bool complete = false;

        if (ImGui::Selectable(".."))
        {
            if (fileTreeState_.currentPath != fileTreeState_.topLevelPath &&
                fileTreeState_.currentPath.has_parent_path())
            {
                fileTreeState_.currentPath = fileTreeState_.currentPath.parent_path();
            }
        }

        for (auto& entry : fs::directory_iterator(fileTreeState_.currentPath))
        {
            const fs::path path = entry.path();
            const std::string name = path.filename().string();
            const bool isDir = entry.is_directory();

            if (isDir)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
            }

            const bool selected = (fileTreeState_.selectedFile.has_value() &&
                                  *fileTreeState_.selectedFile == path);
            const bool activated = ImGui::Selectable(name.c_str(), selected, 
                                                     ImGuiSelectableFlags_DontClosePopups);
            const bool hovered = ImGui::IsItemHovered();
            const bool doubleClicked = ImGui::IsMouseDoubleClicked(0);

            if (hovered)
            {
                if (!isDir)
                {
                    DrawSpriteToolTip(textureRepo, path);
                }
                if (activated || doubleClicked)
                {
                    if (isDir)
                    {
                        // Enter folder
                        fileTreeState_.currentPath = path;
                        fileTreeState_.selectedFile.reset();
                    }
                    else
                    {
                        // Single click: select file
                        fileTreeState_.selectedFile = path;

                        // Double-click: confirm selection Å® return true
                        complete = doubleClicked;
                    }
                }
            }

            if (isDir)
            {
                ImGui::PopStyleColor();
            }
        }

        if (!complete && fileTreeState_.selectedFile.has_value())
        {
            complete = (ImGui::Button("Open"));
        }

        if (complete)
        {
            CompleteSelection(textureRepo);

            isOpen_ = false;
        }
        else
        {
            spriteState_.selectedSprite.reset();
        }

        return complete;
	}

    const std::optional<SpriteData> GetSelectedSprite() const
    {
        return spriteState_.selectedSprite;
    }

    void Reset()
    {
        fileTreeState_.currentPath = fileTreeState_.topLevelPath;
        fileTreeState_.selectedFile.reset();

        spriteState_.hoveredSprite.reset();
        spriteState_.selectedSprite.reset();
        spriteState_.toolTipImage.reset();

        isOpen_ = false;
    }

    bool IsOpen() const { return isOpen_; }

private:

    static Dimensions<float> GetTextureSize(SDL_Texture* tx)
    {
        SDL_Point size{ 0, 0 };
        SDL_QueryTexture(tx, NULL, NULL, &size.x, &size.y);

        return { static_cast<float>(size.x), static_cast<float>(size.y) };
    }

    void CompleteSelection(TextureRepository& textureRepo)
    {
        auto& spriteAtlas = textureRepo.GetSpriteAtlas();
        const auto spriteName = fileTreeState_.selectedFile->stem().string();

        if (!spriteAtlas.HasSprite(spriteName))
        {
            auto loaded = spriteAtlas.LoadSprite(SDLite::Renderer(), { .spriteName = spriteName });
            assert(loaded.Success());
        }

        spriteState_.selectedSprite.emplace(spriteAtlas.GetSprite(spriteName), spriteName);
    }

    void DrawSpriteToolTip(TextureRepository& textureRepo, const fs::path& spritePath)
    {
        auto& spriteAtlas = textureRepo.GetSpriteAtlas();

        const bool proceed = SetSpriteHovered(spriteAtlas, spritePath);
        if (!proceed)
        {
            return;
        }

        UpdateSpriteToolTipImage(textureRepo);
        
        if (spriteState_.toolTipImage.has_value())
        {
            spriteState_.toolTipImage->Draw();
        }
    }

    void UpdateSpriteToolTipImage(TextureRepository& textureRepo)
    {
        if (!spriteState_.hoveredSprite.has_value())
        {
            spriteState_.toolTipImage.reset();
            return;
        }

        const auto& sprite = spriteState_.hoveredSprite->sprite;
        assert(sprite.resourceHandle.IsValid());

        auto* srcTexture = textureRepo.GetSourceTexture(sprite.resourceHandle);
        assert(srcTexture);

        auto& toolTipImage = spriteState_.toolTipImage;

        if (!toolTipImage.has_value())
        {
            const auto [txW, txH] = GetTextureSize(srcTexture);
            assert(txW > 0 && txH > 0);
            const auto [spriteX, spriteY, spriteW, spriteH] = sprite.plot.rect;

            ImVec2 uv0{
                spriteX / txW,
                spriteY / txH
            };

            ImVec2 uv1{
                (spriteX + spriteW) / txW,
                (spriteY + spriteH) / txH
            };

            toolTipImage = {
                0,
                ImVec2{spriteW * 3.0f, spriteH * 3.0f},
                uv0,
                uv1
            };
        }


        assert(toolTipImage.has_value());

        toolTipImage->textureId = (ImTextureID)srcTexture;
    }

    bool SetSpriteHovered(SpriteAtlas& spriteAtlas, const fs::path& spritePath)
    {
        const auto spriteName = spritePath.stem().string();

        // same as last hovered sprite, return
        if (spriteState_.hoveredSprite.has_value() &&
            spriteState_.hoveredSprite->spriteName == spriteName)
        {
            assert(spriteState_.toolTipImage);
            return true;
        }

        // different sprite, reset tooltip image
        spriteState_.toolTipImage.reset();

        // if sprite dir not loaded, load it
        if (!spriteAtlas.HasSprite(spriteName))
        {
            assert(spritePath.has_parent_path());
            const auto dir = spritePath.parent_path();

            bool loaded = LoadSpriteDirectory(spriteAtlas, dir);
            if (!loaded)
            {
                spriteState_.hoveredSprite.reset();
                return false;
            }
        }

        // assign hovered sprite
        auto sprite = spriteAtlas.GetSprite(spriteName);
        assert(sprite.resourceHandle.IsValid());

        spriteState_.hoveredSprite.emplace(sprite, spriteName);

        return true;
    }

    bool LoadSpriteDirectory(SpriteAtlas& spriteAtlas, const fs::path& dir)
    {
        SpriteDescriptors descriptors;
        for (auto& entry : fs::directory_iterator(dir))
        {
            if (entry.is_directory())
            {
                continue;
            }

            descriptors.data.emplace_back(SpriteDescriptor{
               .filepath = entry.path().string()
            });
        }

        if (descriptors.data.empty())
        {
            return false;
        }

        auto result = spriteAtlas.LoadSprites(SDLite::Renderer(), std::move(descriptors));
        if (!result.Success())
        {
            LOG_ERROR(result.GetError());
            return false;
        }

        return true;
    }

    FileTreeState fileTreeState_;
    SpriteState spriteState_;
    bool isOpen_ = false;
};

} // ui


#endif