#pragma once

// TODO : Fix since new texture repo
#if IMGUI_ENABLED

#include "../../atlas/NewTextureRepository.h"
#include "../../gui/GuiContext.h"
#include <filesystem>

struct TextureRepositoryContext
{
	static inline TextureRepository* repo = nullptr;
	static inline std::vector<Sprite> loadedSprites{};
};

class RenderableEditor
{
public:
	static Result<Void> Init(TextureRepository& srcRepo)
	{
		using CTX = TextureRepositoryContext;

		if (CTX::repo)
		{
			LOG_WARNING("TextureRepositoryContext's repo was already assigned");
			return Void{};
		}

		CTX::repo = &srcRepo;
		TRY(LoadTestSprites());

		return Void{};
	}

	static Result<Void> FillSpriteDescriptors(const std::filesystem::path& dirPath,
		SpriteDescriptors& descriptors)
	{
		namespace fs = std::filesystem;

		if (!fs::exists(dirPath))
		{
			return MAKE_ERROR_FMT("Directory path '{}' does not exist", dirPath.string());
		}

		for (const auto& entry : fs::directory_iterator(dirPath))
		{
			if (entry.is_directory())
			{
				TRY(FillSpriteDescriptors(entry, descriptors));
				continue;
			}

			if (entry.is_regular_file())
			{
				const auto& imgPath = entry.path();
				if (imgPath.extension() != ".png")
				{
					continue;
				}

				descriptors.data.push_back(SpriteDescriptor{
					.filepath = imgPath.string()
				});
			}
		}

		return Void{};
	}

	static Result<Void> LoadTestSprites()
	{
		if (!SDLite::Running())
		{
			return MAKE_ERROR("SDL was not started");
		}

		auto dirPath = FilePathUtility::GetRootPath()
			/ kResourcesDirName
			/ kSpritesDirName
			/ "knight";
		if (!fs::exists(dirPath))
		{
			return MAKE_ERROR_FMT("Directory path '{}' does not exist", dirPath.string());
		}

		SpriteDescriptors descriptors;
		TRY(FillSpriteDescriptors(dirPath, descriptors));
		if (descriptors.data.empty())
		{
			return MAKE_ERROR("Descriptors were empty after directory travel");
		}

		using CTX = TextureRepositoryContext;

		if (!CTX::repo)
		{
			return MAKE_ERROR("TextureRepositoryContext's repo has not been set");
		}
		//TRY(CTX::repo->CreateAtlas<SpriteAtlasTexture>(SDLite::Renderer(), 
		//	TextureAtlas::kMaxAtlasSize), spriteAtlas);
		//assert(spriteAtlas);
		auto& spriteAtlas = CTX::repo->GetSpriteAtlas();

		TRY(spriteAtlas.LoadSprites(SDLite::Renderer(), std::move(descriptors)),
			loadedSprites);

		if (loadedSprites.empty())
		{
			return MAKE_ERROR("SpriteAtlas::LoadSprites result sprites were empty");
		}

		return Void{};
	}

	static bool DrawAvailableSprites(Sprite& selectedSprite)
	{
		using CTX = TextureRepositoryContext;

		if (!CTX::repo)
		{
			LOG_ERROR("TextureRepositoryContext's repo has not been set");
			return false;
		}

		bool complete = false;

		//if (!CTX::repo->HasAtlas(CTX::loadedSpriteAtlasHandle))
		//{
		//	ImGui::Text("No sprites loaded!");
		//	return false;
		//}

		const auto& spriteAtlas = CTX::repo->GetSpriteAtlas();

		size_t spriteIdx = 0;
		for (auto& spriteName : spriteAtlas.IterSpriteInfo<&SpriteInfo::spriteName>())
		{
			ImGui::PushID(static_cast<int>(spriteIdx));

			bool selected = false;
			bool activated = ImGui::Selectable(spriteName.c_str(), selected);
			bool doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);

			if (activated || doubleClicked)
			{
				selectedSprite = spriteAtlas.GetSprite(spriteName);
				assert(spriteAtlas.IsSpriteValid(selectedSprite));
				 
				complete = true;
			}

			ImGui::PopID();
		}

		return complete;
	}

private:
};

#endif