#pragma once
#include "../core/TypeUtils.h"
#include "AtlasMap.h"
#include "SpriteSeriesAtlas.h"
#include "GlyphAtlas.h"
#include "SpriteAtlas.h"
#include "NewGlyphAtlas.h"
#include "../components/RenderableComponent.h"

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




//const SDL_Texture* const GetSourceTexture(const NewRenderable& renderable) const
//{
//	return std::visit([this, srcAtlas = renderable.sourceAtlas]
//	(const auto& renderData) {
//			using type = atlas_type_for_render_data_t<raw_type_t<decltype(renderData)>>;
//			auto* atlas = GetAtlas<SpriteAtlas>(srcAtlas);
//			return atlas ? atlas->GetSourceTexture() : nullptr;
//		}, renderable.renderData);
//}
