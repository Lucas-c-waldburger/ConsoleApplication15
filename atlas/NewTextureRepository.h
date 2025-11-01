#pragma once
#include "NewGlyphAtlas.h"
#include "SpriteAtlas.h"
#include "../components/RenderableComponent.h"


//template <typename T>
//const Handle<NewTextureAtlas>& GetRenderDataSourceAtlas(const T& r);
//
//template <typename T>
//concept SomeAtlasTextureAccessType = requires(const T & t) {
//	{ GetRenderDataSourceAtlas(t) } -> std::same_as<const Handle<NewTextureAtlas>&>;
//};

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

//class NewTextureRepository
//{
//public:
//	template <typename T>
//	bool ValidateRenderData(const T& renderData)
//	{
//		const auto& atlasVec = GetAtlasesForRenderData<T>();
//
//	}
//	
//
//private:
//	using AtlasTuple = std::tuple<std::vector<SpriteAtlas>, std::vector<NewGlyphAtlas>>;
//	static constexpr size_t SpriteAtlasIndex = index_of_v<std::vector<SpriteAtlas>, AtlasTuple>;
//	static constexpr size_t GlyphAtlasIndex = index_of_v<std::vector<NewGlyphAtlas>, AtlasTuple>;
//
//	template <typename T>
//	auto& GetAtlasesForRenderData() 
//	{ 
//		using AtlasVecType = std::vector<atlas_type_for_render_data_t<T>>;
//		return std::get<AtlasVecType>(atlases_);
//	}
//
//	template <SomeTextureAtlas T>
//	std::vector<T>& GetAtlasVector() { return std::get<std::vector<T>>(atlases_); }
//
//	template <SomeTextureAtlas T>
//	const std::vector<T>& GetAtlasVector() const { return std::get<std::vector<T>>(atlases_); }
//
//	AtlasTuple atlases_;
//};

class NewTextureRepository
{
private:
	using AtlasVectors = std::tuple<std::vector<SpriteAtlas>,
								    std::vector<NewGlyphAtlas>>;

	struct AtlasIndexData
	{
		size_t typeIndex = kSizeMax;
		size_t vecIndex = kSizeMax;
	};

	template <SomeTextureAtlas T>
	static constexpr size_t GetAtlasTypeIndex()
	{
		return index_of_v<std::vector<T>, AtlasVectors>;
	}

	template <SomeTextureAtlas T>
	static AtlasIndexData MakeIndexData(const std::vector<T>& atlasVec)
	{
		return {
			.typeIndex = GetAtlasTypeIndex<T>(),
			.vecIndex = atlasVec.size()
		};
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
	SDL_Texture* GetTextureImpl(size_t vecIdxToGet) const
	{
		auto& vec = GetAtlasVector<T>();
		if (vec.empty() || vecIdxToGet >= vec.size())
		{
			return nullptr;
		}

		return vec[vecIdxToGet].GetSourceTexture();
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

	template <typename T>
	const Handle<NewTextureAtlas>& GetRenderDataAtlasHandle(const T& renderData)
	{
		if constexpr (std::same_as<T, NewSpriteRenderable>)
		{
			return renderData.sprite.sourceAtlas;
		}
		else if constexpr (std::same_as<T, NewTextRenderable>)
		{
			return renderData.writer.sourceAtlas;
		}
		else { static_assert(false); }
	}

	template <typename T, SomeTextureAtlas U>
	const Handle<NewTextureAtlas>& IsRenderDataValidForAtlas(const T& renderData, const U& atlas)
	{
		if constexpr (std::same_as<T, NewSpriteRenderable> && std::same_as<U, SpriteAtlas>)
		{
			return atlas.IsSpriteValid(renderData.sprite);
		}
		else if constexpr (std::same_as<T, NewTextRenderable> && std::same_as<U, NewGlyphAtlas>)
		{
			return atlas.IsTextWriterValid(renderData.writer);
		}
		else { static_assert(false); }
	}

	template <typename T>
	auto& GetAtlasesForRenderDataType() 
	{ 
		return GetAtlasVector<atlas_type_for_render_data_t<T>>();
	}
	template <typename T>
	const auto& GetAtlasesForRenderDataType() const
	{
		return GetAtlasVector<atlas_type_for_render_data_t<T>>();
	}

public:
	template <typename RenderDataT>
	auto* GetAtlasForRenderData(const RenderDataT& renderData) const
	{
		const auto& handle = GetRenderDataAtlasHandle(renderData);

		auto it = indexMap_.find(handle);
		if (it == indexMap_.end())
		{
			return nullptr;
		}

		using AtlasType = atlas_type_for_render_data_t<RenderDataT>;

		const auto [typeIdx, vecIdx] = it->second;
		if (typeIdx != GetAtlasTypeIndex<AtlasType>())
		{
			return nullptr;
		}

		auto& vec = GetAtlasVector<AtlasType>;
		if (vecIdx > vec.size())
		{
			return nullptr;
		}

		auto& atlas = vec[vecIdx];
		assert(atlas.IsLoaded());
		assert(it->first == atlas.GetHandle());

		return (IsRenderDataValidForAtlas(renderData, atlas)) ? &atlas : nullptr;
	}

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

		return (vecIdx < vec.size()) ? &vec[vecIdx] : nullptr;
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

		return (vecIdx < vec.size()) ? &vec[vecIdx] : nullptr;
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

		auto [_, inserted] = indexMap_.emplace(atlas.GetHandle(), MakeIndexData(vec));
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

	SDL_Texture* GetSourceTexture(const Handle<NewTextureAtlas>& handle) const
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

	template <typename RenderDataT>
	SDL_Texture* GetSourceTexture(const RenderDataT& renderData) const
	{
		auto* srcAtlas = GetAtlasForRenderData(renderData);

		return (srcAtlas) ? srcAtlas->GetSourceTexture() : nullptr;
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

//class NewTextureAtlasActual : public NewTextureAtlas
//{
//public:
//	bool ValidateSprite
//
//private:
//};