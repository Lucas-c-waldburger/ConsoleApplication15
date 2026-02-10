#pragma once
#include "../core/Algorithms.h"
#include "GlyphAtlasCollection.h"
#include "SpriteAtlasCollection.h"
#include "../components/RenderableComponent.h"
#include "AtlasConcepts.h"
#include "../core/Result.h"

//template <typename T>
//concept SomeTextureAtlas = std::derived_from<T, TextureAtlas>;
//
//class TextureRepository
//{
//private:
//	using AtlasVectors = std::tuple<std::vector<SpriteAtlas>,
//								    std::vector<GlyphAtlas>>;
//
//	struct AtlasIndexData
//	{
//		size_t typeIndex = kSizeMax;
//		size_t vecIndex = kSizeMax;
//	};
//
//	template <SomeTextureAtlas T>
//	static constexpr size_t GetAtlasTypeIndex()
//	{
//		return index_of_v<std::vector<T>, AtlasVectors>;
//	}
//
//	template <SomeTextureAtlas T>
//	static AtlasIndexData MakeIndexData(const std::vector<T>& atlasVec)
//	{
//		return {
//			.typeIndex = GetAtlasTypeIndex<T>(),
//			.vecIndex = atlasVec.size()
//		};
//	}
//
//	template <SomeTextureAtlas T>
//	bool DestroyImpl(size_t vecIdxToDestroy)
//	{
//		auto& vec = GetAtlasVector<T>();
//		if (vec.empty() || vecIdxToDestroy >= vec.size())
//		{
//			return false;
//		}
//
//		if (vecIdxToDestroy != vec.size() - 1)
//		{
//			const auto backHandle = vec.back().GetHandle();
//
//			auto it = indexMap_.find(backHandle);
//			assert(it != indexMap_.end());
//
//			it->second.vecIndex = vecIdxToDestroy;
//			std::swap(vec[vecIdxToDestroy], vec[vec.size() - 1]);
//		}
//
//		indexMap_.erase(vec.back().GetHandle());
//		vec.pop_back();
//
//		return true;
//	}
//
//	template <SomeTextureAtlas T>
//	SDL_Texture* GetTextureImpl(size_t vecIdxToGet) const
//	{
//		auto& vec = GetAtlasVector<T>();
//		if (vec.empty() || vecIdxToGet >= vec.size())
//		{
//			return nullptr;
//		}
//
//		return vec[vecIdxToGet].GetSourceTexture();
//	}
//
//	bool ValidHandle(const Handle<TextureAtlas>& handle) const
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return false;
//		}
//
//		const auto [typeIdx, vecIdx] = it->second;
//
//		switch (typeIdx)
//		{
//		case GetAtlasTypeIndex<SpriteAtlas>():
//			return vecIdx < GetAtlasVector<SpriteAtlas>().size();
//
//		case GetAtlasTypeIndex<GlyphAtlas>():
//			return vecIdx < GetAtlasVector<GlyphAtlas>().size();
//
//		default:
//			return false;
//		}
//	}
//
//	GlyphAtlas* FindGlyphAtlasByFontName(std::string_view fontName)
//	{
//		auto& glyphAtlases = GetAtlasVector<GlyphAtlas>();
//
//		auto it = core::FindIf(glyphAtlases, [fontName](const auto& atlas) {
//			return atlas.GetFontDescriptor().fontName == fontName;
//		});
//
//		return (it != glyphAtlases.end()) ? &(*it) : nullptr;
//	}
//
//public:
//	template <SomeTextureAtlas T>
//	T* GetAtlas(const Handle<TextureAtlas>& handle)
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return nullptr;
//		}
//
//		const auto [typeIdx, vecIdx] = it->second;
//		if (typeIdx != GetAtlasTypeIndex<T>())
//		{
//			return nullptr;
//		}
//
//		auto& vec = GetAtlasVector<T>();
//
//		return (vecIdx < vec.size()) ? &vec[vecIdx] : nullptr;
//	}
//
//	template <SomeTextureAtlas T>
//	const T* GetAtlas(const Handle<TextureAtlas>& handle) const
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return nullptr;
//		}
//
//		const auto [typeIdx, vecIdx] = it->second;
//		if (typeIdx != GetAtlasTypeIndex<T>())
//		{
//			return nullptr;
//		}
//
//		auto& vec = GetAtlasVector<T>();
//
//		return (vecIdx < vec.size()) ? &vec[vecIdx] : nullptr;
//	}
//
//	template <SomeTextureAtlas T, typename... Args> requires 
//		requires(SDL_Renderer* r, Args&&... a) {
//			{ T::Create(r, std::forward<Args>(a)...) };
//		}
//	Result<T*> CreateAtlas(SDL_Renderer* renderer, Args&&...args)
//	{
//		auto newAtlas = T::Create(renderer, std::forward<Args>(args)...);
//		if (!newAtlas.Success())
//		{
//			return newAtlas.GetError();
//		}
//
//		auto& vec = GetAtlasVector<T>();
//		auto [_, inserted] = indexMap_.emplace(newAtlas.GetValue().GetHandle(), 
//											   MakeIndexData(vec));
//		assert(inserted);
//
//		auto& stored = vec.emplace_back(std::move(newAtlas.GetValue()));
//
//		return &stored;
//	}
//
//	template <SomeTextureAtlas T>
//	Result<T*> AttachAtlas(T&& atlas)
//	{
//		if (!atlas.IsLoaded())
//		{
//			return MAKE_ERROR("Atlas was not loaded");
//		}
//		if (indexMap_.contains(atlas.GetHandle()))
//		{
//			return MAKE_ERROR("Duplicate atlas handle");
//		}
//
//		auto& vec = GetAtlasVector<T>();
//
//		auto [_, inserted] = indexMap_.emplace(atlas.GetHandle(), MakeIndexData(vec));
//		assert(inserted);
//
//		auto& stored = vec.emplace_back(std::move(atlas));
//
//		return &stored;
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
//		const auto [typeIdx, vecIdx] = it->second;
//
//		switch (typeIdx)
//		{
//		case GetAtlasTypeIndex<SpriteAtlas>():
//			return DestroyImpl<SpriteAtlas>(vecIdx);
//
//		case GetAtlasTypeIndex<GlyphAtlas>():
//			return DestroyImpl<GlyphAtlas>(vecIdx);
//
//		default:
//			return false;
//		}
//	}
//
//	SDL_Texture* GetSourceTexture(const Handle<TextureAtlas>& handle) const
//	{
//		auto it = indexMap_.find(handle);
//		if (it == indexMap_.end())
//		{
//			return nullptr;
//		}
//
//		const auto [typeIdx, vecIdx] = it->second;
//
//		switch (typeIdx)
//		{
//		case GetAtlasTypeIndex<SpriteAtlas>():
//			return GetTextureImpl<SpriteAtlas>(vecIdx);
//
//		case GetAtlasTypeIndex<GlyphAtlas>():
//			return GetTextureImpl<GlyphAtlas>(vecIdx);
//
//		default:
//			return nullptr;
//		}
//	}
//
//	Result<Void> RebuildAtlasTextures(SDL_Renderer* renderer)
//	{
//		for (auto& spriteAtlas : GetAtlasVector<SpriteAtlas>())
//		{
//			TRY(spriteAtlas.RebuildSourceTexture(renderer));
//		}
//		return Void{};
//	}
//
//	template <SomeTextureAtlas T>
//	const std::vector<T>& GetAtlasVector() const
//	{
//		return std::get<std::vector<T>>(atlases_);
//	}
//
//	bool HasAtlas(const Handle<TextureAtlas>& handle) const
//	{
//		return ValidHandle(handle);
//	}
//
//private:
//	template <SomeTextureAtlas T>
//	std::vector<T>& GetAtlasVector()
//	{
//		return std::get<std::vector<T>>(atlases_);
//	}
//
//	template <SomeTextureAtlas T>
//	bool VecIndexInRange(size_t vecIdx) const
//	{
//		return vecIdx < GetAtlasVector<T>().size();
//	}
//
//	using AtlasIndexMap = std::unordered_map<Handle<TextureAtlas>,
//											 AtlasIndexData>;
//	AtlasIndexMap indexMap_;
//	AtlasVectors atlases_;
//};


class TextureRepository : public TextureObserver
{
public:
	using TextureMap = std::unordered_map<TextureAtlasID, SDL_Texture*>;

	TextureRepository();
	~TextureRepository();

	TextureRepository(const TextureRepository&) = delete;
	TextureRepository& operator=(const TextureRepository&) = delete;

	TextureRepository(TextureRepository&&) noexcept = default;
	TextureRepository& operator=(TextureRepository&&) noexcept = default;

	SpriteAtlas& GetSpriteAtlas() { return spriteAtlasCollection_; }
	const SpriteAtlas& GetSpriteAtlas() const { return spriteAtlasCollection_; }

	FontAtlas& GetFontAtlas() { return glyphAtlasCollection_; }
	const FontAtlas& GetFontAtlas() const { return glyphAtlasCollection_; }

	SDL_Texture* GetSourceTexture(const Handle<TextureResource>& handle) const;

	Result<Void> RebuildSourceTextures(SDL_Renderer* renderer);

private:
	struct Tokens
	{
		SignalToken spriteTextureCreated;
		SignalToken glyphTextureCreated;
		SignalToken rebuildTextures;
	};

	auto GetTextureCreatedCallback()
	{
		return [this](TextureAtlasID atlasId, SDL_Texture* texture) 
		{
			sourceTextureMap_[atlasId] = texture;
		};
	}

	SpriteAtlas spriteAtlasCollection_;
	FontAtlas glyphAtlasCollection_;
	Tokens tokens_;
	TextureMap sourceTextureMap_;
};