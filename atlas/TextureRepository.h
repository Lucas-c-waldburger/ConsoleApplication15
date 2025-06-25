#pragma once
#include "AtlasMap.h"

class TextureRepository
{
public:
	template <SomeTextureAtlas T>
	Result<Handle<T>> LoadAtlas(SDL_Renderer* renderer, AtlasInfo<T> info)
	{
		if constexpr (std::same_as<T, SpriteSeriesAtlas>)
		{
			return spriteAtlases_.LoadAtlas(renderer, std::move(info));
		}
		else if constexpr (std::same_as<T, GlyphAtlas>)
		{
			return glyphAtlases_.LoadAtlas(renderer, std::move(info));
		}
		else
		{
			return MAKE_ERROR("Unrecognized atlas type");
		}
	}

	template <SomeTextureAtlas T>
	SDL_Texture* GetAtlasTexture(const Handle<T>& handle) const
	{
		if constexpr (std::same_as<T, SpriteSeriesAtlas>)
		{
			return spriteAtlases_.GetAtlasTexture(handle);
		}
		else if constexpr (std::same_as<T, GlyphAtlas>)
		{
			return glyphAtlases_.GetAtlasTexture(handle);
		}
		else
		{
			return MAKE_ERROR("Unrecognized atlas type");
		}
	}

	template <SomeTextureAtlas T>
	const T* GetAtlas(const Handle<T>& handle) const
	{
		if constexpr (std::same_as<T, SpriteSeriesAtlas>)
		{
			return spriteAtlases_.GetAtlas(handle);
		}
		else if constexpr (std::same_as<T, GlyphAtlas>)
		{
			return glyphAtlases_.GetAtlas(handle);
		}
		else
		{
			return MAKE_ERROR("Unrecognized atlas type");
		}
	}

private:
	AtlasMap<SpriteSeriesAtlas> spriteAtlases_;
	AtlasMap<GlyphAtlas> glyphAtlases_;
};