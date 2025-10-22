#pragma once
#include "AtlasMap.h"
#include "SpriteSeriesAtlas.h"
#include "GlyphAtlas.h"
#include "SpriteAtlas.h"
#include "NewGlyphAtlas.h"

template <typename T>
concept SupportedAtlasType = std::same_as<T, SpriteSeriesAtlas> || std::same_as<T, GlyphAtlas>;

class TextureRepository
{
public:
	template <SupportedAtlasType T, typename LoadData>
	Result<Handle<T>> LoadNewAtlas(SDL_Renderer* renderer, LoadData&& loadData)
	{ 
		return std::get<AtlasMap<T>>(atlasMaps_).LoadNewAtlas(renderer, std::forward<LoadData>(loadData));
	}

	template <SupportedAtlasType T, typename LoadData>
	Result<Handle<T>> ReloadAtlas(const Handle<T>& handle, SDL_Renderer* renderer, LoadData&& loadData)
	{
		return std::get<AtlasMap<T>>(atlasMaps_).ReloadAtlas(handle, renderer, std::forward<LoadData>(loadData));
	}

	template <SupportedAtlasType T>
	bool RemoveAtlas(const Handle<T>& handle) const
	{
		return std::get<AtlasMap<T>>(atlasMaps_).RemoveAtlas();
	}

	template <SupportedAtlasType T>
	SDL_Texture* GetAtlasTexture(const Handle<T>& handle) const
	{
		return std::get<AtlasMap<T>>(atlasMaps_).GetAtlasTexture(handle);
	}

	template <SupportedAtlasType T>
	const T* GetAtlas(const Handle<T>& handle) const
	{
		return std::get<AtlasMap<T>>(atlasMaps_).GetAtlas(handle);
	}

private:
	std::tuple<AtlasMap<SpriteSeriesAtlas>, AtlasMap<GlyphAtlas>> atlasMaps_;
};


template <typename T>
concept SomeTextureAtlas = std::derived_from<T, NewTextureAtlas>;

using TextureAtlasVariant = std::variant<SpriteAtlas, NewGlyphAtlas>;
using TextureAtlasMap = std::unordered_map<int, TextureAtlasVariant>;

struct TextureTag
{
	int atlasId = 0;
	int plotIndex = 0;
	TextureType textureType = TextureType::Unknown;
};

class NewTextureRepository
{
public:


	SDL_Texture* const GetSourceTexture(const TextureTag& textureTag) const
	{
		auto it = atlasMap_.find(textureTag.atlasId);
		if (it == atlasMap_.end())
		{
			return nullptr;
		}

		return std::visit([idk = textureTag.plotIndex](auto& atlas) {
			return atlas.GetSourceTexture(); 
		}, it->second);
	}

private:
	TextureAtlasMap atlasMap_;
};
