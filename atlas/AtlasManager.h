#pragma once
#include "GlyphAtlas.h"
#include "SpriteSeriesAtlas.h"
#include "../core/HandleFactory.h"
#include "../core/TypeUtils.h"

// TODO: Change file name to AtlasStore.h

struct SDL_Renderer;

template <typename T>
concept SomeTextureAtlas = requires(T atlas, SDL_Renderer* renderer, AtlasInfo<T> info) {
	{ atlas.GetHandle() } -> std::same_as<const Handle<T>&>;
	{ atlas.Load(renderer, info) } -> std::same_as<bool>;
};

template <SomeTextureAtlas...AtlasTs> requires pack_types_unique_v<AtlasTs...>
class TextureManagerTemplate
{
public:
	TextureManagerTemplate() = default;

	template <SomeTypeInPack<AtlasTs...> T>
	Result<Handle<T>> LoadAtlas(SDL_Renderer* renderer, AtlasInfo<T> info)
	{
		auto& atlasMap = GetEntry<T>();

		Handle<T> atlasHandle = handleFactory_.GetHandle<T>();

		auto [atlas, inserted] = atlasMap.emplace(atlasHandle, atlasHandle);
		assert(inserted);

		bool loadResult = atlas->second.Load(renderer, std::move(info));
		if (!loadResult)
		{
			return MAKE_ERROR("Atlas load failed");
		}

		return atlasHandle;
	}

	template <SomeTypeInPack<AtlasTs...> T>
	const T* GetAtlas(const Handle<T>& handle) const
	{
		if (!handleFactory_.IsHandleValid(handle))
		{
			return nullptr;
		}

		const auto& entry = GetEntry<T>();

		auto it = entry.find(handle);

		return (it != entry.end()) ? &it->second : nullptr;
	}

protected:
	template <SomeTypeInPack<AtlasTs...> T>
	auto& GetEntry()
	{
		return std::get<std::unordered_map<Handle<T>, T>>(atlasMaps_);
	}
	template <SomeTypeInPack<AtlasTs...> T>
	const auto& GetEntry() const
	{
		return std::get<std::unordered_map<Handle<T>, T>>(atlasMaps_);
	}

	std::tuple<std::unordered_map<Handle<AtlasTs>, AtlasTs>...> atlasMaps_;
	HandleFactory<AtlasTs...> handleFactory_;
};

// IMPL //
namespace impl {
class TextureManager : public TextureManagerTemplate<GlyphAtlas, SpriteSeriesAtlas> {};
}