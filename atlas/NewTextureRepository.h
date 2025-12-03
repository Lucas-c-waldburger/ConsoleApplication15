#pragma once
#include "../core/Algorithms.h"
#include "NewGlyphAtlas.h"
#include "SpriteAtlas.h"
#include "../components/RenderableComponent.h"
#include "AtlasConcepts.h"
#include "../core/Result.h"

template <typename T>
concept SomeTextureAtlas = std::derived_from<T, TextureAtlas>;

class TextureRepository
{
private:
	using AtlasVectors = std::tuple<std::vector<SpriteAtlas>,
								    std::vector<GlyphAtlas>>;

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

	bool ValidHandle(const Handle<TextureAtlas>& handle) const
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

		case GetAtlasTypeIndex<GlyphAtlas>():
			return vecIdx < GetAtlasVector<GlyphAtlas>().size();

		default:
			return false;
		}
	}

	GlyphAtlas* FindGlyphAtlasByFontName(std::string_view fontName)
	{
		auto& glyphAtlases = GetAtlasVector<GlyphAtlas>();

		auto it = core::FindIf(glyphAtlases, [fontName](const auto& atlas) {
			return atlas.GetFontDescriptor().fontName == fontName;
		});

		return (it != glyphAtlases.end()) ? &(*it) : nullptr;
	}

public:
	template <SomeTextureAtlas T>
	T* GetAtlas(const Handle<TextureAtlas>& handle)
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
	const T* GetAtlas(const Handle<TextureAtlas>& handle) const
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

	template <SomeTextureAtlas T, typename... Args> requires 
		requires(SDL_Renderer* r, Args&&... a) {
			{ T::Create(r, std::forward<Args>(a)...) };
		}
	Result<T*> CreateAtlas(SDL_Renderer* renderer, Args&&...args)
	{
		auto newAtlas = T::Create(renderer, std::forward<Args>(args)...);
		if (!newAtlas.Success())
		{
			return newAtlas.GetError();
		}

		auto& vec = GetAtlasVector<T>();
		auto [_, inserted] = indexMap_.emplace(newAtlas.GetValue().GetHandle(), 
											   MakeIndexData(vec));
		assert(inserted);

		auto& stored = vec.emplace_back(std::move(newAtlas.GetValue()));

		return &stored;
	}

	template <SomeTextureAtlas T>
	Result<T*> AttachAtlas(T&& atlas)
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

		auto& stored = vec.emplace_back(std::move(atlas));

		return &stored;
	}

	bool DestroyAtlas(const Handle<TextureAtlas>& handle)
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

		case GetAtlasTypeIndex<GlyphAtlas>():
			return DestroyImpl<GlyphAtlas>(vecIdx);

		default:
			return false;
		}
	}

	SDL_Texture* GetSourceTexture(const Handle<TextureAtlas>& handle) const
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

		case GetAtlasTypeIndex<GlyphAtlas>():
			return GetTextureImpl<GlyphAtlas>(vecIdx);

		default:
			return nullptr;
		}
	}

	template <SomeTextureAtlas T>
	const std::vector<T>& GetAtlasVector() const
	{
		return std::get<std::vector<T>>(atlases_);
	}

	bool HasAtlas(const Handle<TextureAtlas>& handle) const
	{
		return ValidHandle(handle);
	}

private:
	template <SomeTextureAtlas T>
	std::vector<T>& GetAtlasVector()
	{
		return std::get<std::vector<T>>(atlases_);
	}

	template <SomeTextureAtlas T>
	bool VecIndexInRange(size_t vecIdx) const
	{
		return vecIdx < GetAtlasVector<T>().size();
	}

	using AtlasIndexMap = std::unordered_map<Handle<TextureAtlas>,
											 AtlasIndexData>;
	AtlasIndexMap indexMap_;
	AtlasVectors atlases_;
};


//struct AtlasIndex
//{
//	size_t vectorIndex = kSizeMax;
//	size_t typeIndex = kSizeMax;
//
//	template <SomeTextureAtlasType T>
//	static constexpr AtlasIndex Create(size_t vecIdx)
//	{
//		return AtlasIndex{
//			.vectorIndex = vecIdx,
//			.typeIndex = atlas_storage_index_v<T>
//		};
//	}
//};
//
//using AtlasIndexMap = std::unordered_map<Handle<TextureAtlas>, AtlasIndex>;
//
//namespace detail {
//
//struct atlas_storage_helper
//{
//	template <typename Ret, typename...Args, size_t I = 0> 
//		requires (!std::is_reference_v<Ret>)
//	static auto call_base_fn(AtlasStorage& atlases, const AtlasIndex& atlasIndex,
//						     Ret(TextureAtlas::* fn)(Args...), Args&&... args) 
//	-> add_result_t<Ret>
//	{
//		if constexpr (I < std::tuple_size_v<AtlasStorage>) 
//		{
//			if (atlasIndex.typeIndex == I) 
//			{
//				auto& vec = std::get<I>(atlases);
//				assert(atlasIndex.vectorIndex < vec.size());
//
//				if constexpr (std::is_void_v<Ret>)
//				{
//					static_cast<TextureAtlas&>(vec[atlasIndex.vectorIndex]).*fn(
//											   std::forward<Args>(args)...);
//					return Void{};
//				}
//				else
//				{
//					return static_cast<TextureAtlas&>(vec[atlasIndex.vectorIndex]).*fn(
//													  std::forward<Args>(args)...);
//				}
//			}
//			else 
//			{
//				return call_base_fn<I + 1>(atlases, atlasIndex, fn, std::forward<Args>(args)...);
//			}
//		}
//		else 
//		{
//			return MAKE_ERROR_FMT("Invalid AtlasIndex. vectorIndex: '{}', typeIndex: '{}'",
//				atlasIndex.vectorIndex, atlasIndex.typeIndex);
//		}
//	}
//
//	template <typename Ret, typename...Args, size_t I = 0>
//		requires (!std::is_reference_v<Ret>)
//	static auto call_base_fn(const AtlasStorage& atlases, const AtlasIndex& atlasIndex,
//							 Ret(TextureAtlas::* fn)(Args...) const, Args&&... args)
//	-> add_result_t<Ret>
//	{
//		if constexpr (I < std::tuple_size_v<AtlasStorage>)
//		{
//			if (atlasIndex.typeIndex == I)
//			{
//				auto& vec = std::get<I>(atlases);
//				assert(atlasIndex.vectorIndex < vec.size());
//
//				if constexpr (std::is_void_v<Ret>)
//				{
//					static_cast<const TextureAtlas&>(vec[atlasIndex.vectorIndex])
//						.*fn(std::forward<Args>(args)...);
//
//					return Void{};
//				}
//				else
//				{
//					return static_cast<const TextureAtlas&>(vec[atlasIndex.vectorIndex])
//						.*fn(std::forward<Args>(args)...);
//				}
//			}
//			else
//			{
//				return call_base_fn<I + 1>(atlases, atlasIndex, fn, std::forward<Args>(args)...);
//			}
//		}
//		else
//		{
//			return add_result_t<Ret>(
//				MAKE_ERROR_FMT("Invalid AtlasIndex. vectorIndex: '{}', typeIndex: '{}'",
//					atlasIndex.vectorIndex, atlasIndex.typeIndex)
//			);
//		}
//	}
//
//	template <size_t I = 0>
//	static bool destroy_atlas(AtlasStorage& atlases, const AtlasIndex& atlasIndex,
//							  AtlasIndexMap& indexMap)
//	{
//		if constexpr (I < std::tuple_size_v<AtlasStorage>)
//		{
//			if (atlasIndex.typeIndex == I)
//			{
//				auto& vec = std::get<I>(atlases);
//
//				if (atlasIndex.vectorIndex >= vec.size())
//				{
//					return false;
//				}
//
//				const auto& backHandle = vec.back().GetHandle();
//
//				auto it = indexMap.find(backHandle);
//				assert(it != indexMap.end());
//				assert(it->second.typeIndex == I)
//				assert(it->second.vectorIndex == vec.size() - 1);
//
//				std::swap(vec[it->second.vectorIndex], vec[atlasIndex.vectorIndex]);
//				it->second.vectorIndex = atlasIndex.vectorIndex;
//
//				vec.pop_back();
//
//				return true;
//			}
//			else
//			{
//				return destroy_atlas<I + 1>(atlases, atlasIndex, indexMap);
//			}
//		}
//		else
//		{
//			return false;
//		}
//	}
//};
//
//template <typename Tup, typename Fn, typename...Args, std::size_t I = 0>
//decltype(auto) dispatch_by_index(Tup& tup, std::size_t index, Fn&& fn, Args&&...args)
//{
//	if constexpr (I < std::tuple_size_v<Tup>)
//	{
//		if (index == I)
//		{
//			return std::invoke(fn, std::get<I>(tup));
//		}
//		else
//		{
//			return dispatch_by_index<I + 1>(tup, index, 
//				std::forward<Fn>(fn), std::forward<Args>(args)...);
//		}
//	}
//}
//
//} // detail

//class NewTextureRepository
//{
//private:
//	template <size_t I = 0>
//	Result<SDL_Texture*> GetSourceTextureImpl(AtlasIndex atlasIdx) const
//	{
//		if (atlasIdx.typeIndex == I)
//		{
//			auto& vec = std::get<I>(atlases_);
//			assert(atlasIdx.vectorIndex < vec.size());
//
//			return vec[atlasIdx.vectorIndex].GetSourceTexture();
//		}
//		else
//		{
//			return GetSourceTextureImpl<I + 1>(atlasIdx);
//		}
//	}
//
//	template <size_t I = 0>
//	void DestroyAtlasImpl(AtlasIndex atlasIdx)
//	{
//		if (atlasIdx.typeIndex == I)
//		{
//			auto& vec = std::get<I>(atlases_);
//			assert(atlasIdx.vectorIndex < vec.size());
//
//			const auto& backHandle = vec.back().GetHandle();
//
//			auto it = indexMap_.find(backHandle);
//			assert(it != indexMap_.end());
//			assert(it->second.typeIndex == I);
//			assert(it->second.vectorIndex == vec.size() - 1);
//
//			if (atlasIdx.vectorIndex != vec.size() - 1)
//			{
//				std::swap(vec[it->second.vectorIndex], vec[atlasIdx.vectorIndex]);
//				it->second.vectorIndex = atlasIdx.vectorIndex;
//			}
//
//			vec.pop_back();
//		}
//		else
//		{
//			DestroyAtlasImpl<I + 1>(atlasIdx);
//		}
//	}
//
//public:
//	Result<SDL_Texture*> GetSourceTexture(const Handle<TextureAtlas>& handle) const
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return MAKE_ERROR("Texture Atlas Handle not found in Texture Repository");
//		}
//		assert(it->second.typeIndex < std::tuple_size_v<AtlasStorage>);
//
//		return GetSourceTextureImpl(it->second);
//	}
//
//	template <SomeTextureAtlasType T>
//	Result<T*> GetAtlas(const Handle<TextureAtlas>& handle)
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return MAKE_ERROR("Texture Atlas Handle not found in Texture Repository");
//		}
//		if (it->second.typeIndex != atlas_storage_index_v<T>)
//		{
//			return MAKE_ERROR("Texture Atlas Handle is for a different atlas type");
//		}
//
//		auto& vec = GetAtlasVector<T>();
//		assert(it->second.vectorIndex < vec.size());
//
//		return &(vec[it->second.vectorIndex]); 
//	}
//
//	template <SomeTextureAtlasType T>
//	Result<const T*> GetAtlas(const Handle<TextureAtlas>& handle) const
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return MAKE_ERROR("Texture Atlas Handle not found in Texture Repository");
//		}
//		if (it->second != atlas_storage_index_v<T>)
//		{
//			return MAKE_ERROR("Texture Atlas Handle is for a different atlas type");
//		}
//
//		const auto& vec = GetAtlasVector<T>();
//		assert(it->second.vectorIndex < vec.size());
//
//		return &(vec[it->second.vectorIndex]);
//	}
//
//	template <SomeTextureAtlasType T, typename...Args>
//		requires std::invocable<decltype(T::Create), SDL_Renderer*, Args...>
//	Result<T*> CreateAtlas(SDL_Renderer* renderer, Args&&...args)
//	{
//		auto newAtlas = T::Create(renderer, std::forward<Args>(args)...);
//		if (!newAtlas.Success())
//		{
//			return newAtlas.GetError();
//		}
//
//		auto& vec = GetAtlasVector<T>();
//		indexMap_[newAtlas.GetValue().GetHandle()] = AtlasIndex::Create<T>(vec.size());
//
//		auto& ref = vec.emplace_back(std::move(newAtlas.GetValue()));
//
//		return &ref;
//	}
//
//	template <SomeTextureAtlasType T>
//	Result<Handle<TextureAtlas>> AttachAtlas(T&& atlas)
//	{
//		if (!atlas.IsLoaded())
//		{
//			return MAKE_ERROR("Atlas was not loaded");
//		}
//
//		auto handle = atlas.GetHandle();
//		assert(handle.IsValid());
//
//		if (indexMap_.contains(handle))
//		{
//			return MAKE_ERROR("Atlas handle already registered with Texture Repository");
//		}
//
//		auto& vec = GetAtlasVector<T>();
//		indexMap_[handle] = AtlasIndex::Create<T>(vec.size());
//
//		vec.emplace_back(std::move(atlas));
//
//		return handle;
//	}
//
//	bool DestroyAtlas(const Handle<TextureAtlas>& handle)
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return false;
//		}
//
//		assert(it->second.typeIndex < std::tuple_size_v<AtlasStorage>);
//
//		DestroyAtlasImpl(it->second);
//
//		return true;
//	}
//
//private:
//	template <SomeTextureAtlasType T>
//	auto& GetAtlasVector() { return std::get<std::vector<T>>(atlases_); }
//
//	template <SomeTextureAtlasType T>
//	const auto& GetAtlasVector() const { return std::get<std::vector<T>>(atlases_); }
//
//	AtlasIndexMap indexMap_;
//	AtlasStorage atlases_;
//};