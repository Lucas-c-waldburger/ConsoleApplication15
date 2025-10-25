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


template <typename T>
concept SomeTextureAtlas = std::derived_from<T, NewTextureAtlas>;

using TextureAtlasVariant = std::variant<SpriteAtlas, NewGlyphAtlas>;
using TextureAtlasMap = std::unordered_map<Handle<NewTextureAtlas>, 
										   TextureAtlasVariant>;

namespace detail {
template <typename T> 
struct atlas_type_for_render_data;
template <> 
struct atlas_type_for_render_data<NewSpriteRenderable> {
	using type = SpriteAtlas;
};
template <>
struct atlas_type_for_render_data<NewTextRenderable> {
	using type = NewGlyphAtlas;
};
} // detail

template <typename T>
using atlas_type_for_render_data_t = detail::atlas_type_for_render_data<T>::type;

class NewTextureRepository
{
private:
	using AtlasVectors = std::tuple<std::vector<SpriteAtlas>,
									std::vector<NewGlyphAtlas>>;

	template <SomeTextureAtlas T>
	static constexpr size_t GetAtlasTypeIndex()
	{
		return index_of_v<std::vector<T>, AtlasVectors>;
	}

	struct AtlasIndexData
	{
		size_t typeIndex = kSizeMax;
		size_t vecIndex = kSizeMax;
	};

	template <SomeTextureAtlas T>
	static AtlasIndexData MakeIndexData(const std::vector<T>& atlasVec)
	{
		return {
			.typeIndex = GetAtlasTypeIndex<T>(),
			.vecIndex = atlasVec.size()
		});
	}

	template <SomeTextureAtlas T>
	bool DestroyImpl(size_t vecIdxToDestroy)
	{
		auto& vec = GetAtlasVector<T>();
		if (vec.empty() || vecIdxToDestroy >= vec.size())
		{
			return false;
		}

		if (vecIdxToDestroy != vec.size() - 1)
		{
			const auto backHandle = vec.back().GetHandle();

			auto it = indexMap_.find(backHandle);
			assert(it != indexMap_.end());

			it->second.vecIndex = vecIdxToDestroy;
			std::swap(vec[vecIdxToDestroy], vec[vec.size() - 1]);
		}

		indexMap_.erase(vec.back().GetHandle());
		vec.pop_back();

		return true;
	}

	template <SomeTextureAtlas T>
	const SDL_Texture* GetTextureImpl(size_t vecIdxToGet) const
	{
		auto& vec = GetAtlasVector<T>();
		if (vec.empty() || vecIdxToGet >= vec.size())
		{
			return nullptr;
		}

		return vec[vecIdx].GetSourceTexture();
	}

	bool ValidHandle(const Handle<NewTextureAtlas>& handle) const
	{
		auto it = indexMap_.find(handle);
		if (it == indexMap_.end())
		{
			return false;
		}

		const auto [typeIdx, vecIdx] = it->second;

		switch (typeIdx)
		{
		case GetAtlasTypeIndex<SpriteAtlas>():
			return vecIdx < GetAtlasVector<SpriteAtlas>().size();

		case GetAtlasTypeIndex<NewGlyphAtlas>():
			return vecIdx < GetAtlasVector<NewGlyphAtlas>().size();

		default:
			return false;
		}
	}

public:
	template <SomeTextureAtlas T>
	T* GetAtlas(const Handle<NewTextureAtlas>& handle)
	{
		auto it = indexMap_.find(handle);
		if (it == indexMap_.end())
		{
			return nullptr;
		}

		const auto [typeIdx, vecIdx] = it->second;
		if (typeIdx != GetAtlasTypeIndex<T>())
		{
			return nullptr;
		}

		auto& vec = GetAtlasVector<T>();

		return (vecIdx < vec.size()) ? vec[vecIdx] : nullptr;
	}

	template <SomeTextureAtlas T>
	const T* GetAtlas(const Handle<NewTextureAtlas>& handle) const
	{
		auto it = indexMap_.find(handle);
		if (it == indexMap_.end())
		{
			return nullptr;
		}

		const auto [typeIdx, vecIdx] = it->second;
		if (typeIdx != GetAtlasTypeIndex<T>())
		{
			return nullptr;
		}

		auto& vec = GetAtlasVector<T>();

		return (vecIdx < vec.size()) ? vec[vecIdx] : nullptr;
	}

	template <SomeTextureAtlas T>
	Result<Void> AttachAtlas(T&& atlas)
	{
		if (!atlas.IsLoaded())
		{
			return MAKE_ERROR("Atlas was not loaded");
		}
		if (indexMap_.contains(atlas.GetHandle()))
		{
			return MAKE_ERROR("Duplicate atlas handle");
		}

		auto& vec = GetAtlasVector<T>();

		bool[_, inserted] = indexMap_.emplace(atlas.GetHandle(), MakeIndexData(vec));
		assert(inserted);

		vec.emplace_back(std::move(atlas));

		return Void{};
	}

	bool DestroyAtlas(const Handle<NewTextureAtlas>& handle)
	{
		auto it = indexMap_.find(handle);
		if (it == indexMap_.end())
		{
			return false;
		}

		const auto [typeIdx, vecIdx] = it->second;

		switch (typeIdx)
		{
		case GetAtlasTypeIndex<SpriteAtlas>():
			return DestroyImpl<SpriteAtlas>(vecIdx);

		case GetAtlasTypeIndex<NewGlyphAtlas>():
			return DestroyImpl<NewGlyphAtlas>(vecIdx);

		default:
			return false;
		}
	}

	const SDL_Texture* GetSourceTexture(const Handle<NewTextureAtlas>& handle) const
	{
		auto it = indexMap_.find(handle);
		if (it == indexMap_.end())
		{
			return nullptr;
		}

		const auto [typeIdx, vecIdx] = it->second;

		switch (typeIdx)
		{
		case GetAtlasTypeIndex<SpriteAtlas>():
			return GetTextureImpl<SpriteAtlas>(vecIdx);

		case GetAtlasTypeIndex<NewGlyphAtlas>():
			return GetTextureImpl<NewGlyphAtlas>(vecIdx);

		default:
			return nullptr;
		}
	}


private:
	template <SomeTextureAtlas T>
	std::vector<T>& GetAtlasVector()
	{
		return std::get<std::vector<T>>(atlases_);
	}
	template <SomeTextureAtlas T>
	const std::vector<T>& GetAtlasVector() const
	{
		return std::get<std::vector<T>>(atlases_);
	}

	template <SomeTextureAtlas T>
	bool VecIndexInRange(size_t vecIdx) const
	{
		return vecIdx < GetAtlasVector<T>().size();
	}

	using AtlasIndexMap = std::unordered_map<Handle<NewTextureAtlas>,
											 AtlasIndexData>;

	AtlasIndexMap indexMap_;
	AtlasVectors atlases_;
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
