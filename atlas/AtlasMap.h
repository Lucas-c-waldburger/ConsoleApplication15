#pragma once
#include "AtlasManager.h"

// TODO: Clearing Atlas, isLoaded flag, reusing unloaded atlases

template <SomeTextureAtlas T>
class AtlasMap
{
public:
	Result<Handle<T>> LoadAtlas(SDL_Renderer* renderer, AtlasInfo<T>&& info)
	{
		Handle<T> atlasHandle = Handle<T>::Create();

		auto [atlas, inserted] = map_.emplace(atlasHandle, atlasHandle);
		assert(inserted);

		bool loadResult = atlas->second.Load(renderer, std::move(info));
		if (!loadResult)
		{
			map_.erase(atlas);

			return MAKE_ERROR("Atlas load failed");
		}

		return atlasHandle;
	}

	SDL_Texture* GetAtlasTexture(const Handle<T>& handle) const
	{
		auto it = map_.find(handle);

		return (it != map_.end()) ? it->second.GetAtlasTexture() : nullptr;
	}

	const T* GetAtlas(const Handle<T>& handle) const
	{
		auto it = map_.find(handle);

		return (it != map_.end()) ? &it->second : nullptr;
	}

private:
	std::unordered_map<Handle<T>, T> map_;
};

template <typename T>
class AtlasMap2
{
public:
	template <typename LoadData>
	Result<Handle<T>> LoadAtlas(SDL_Renderer* renderer, LoadData&& data)
	{
		auto& newAtlas = atlases_.emplace_back();

		auto loadResult = newAtlas.Load(renderer, std::move(data));
		if (!loadResult.Success())
		{
			atlases_.pop_back();

			return loadResult.GetError();
		}

		auto handle = loadResult.GetValue();
		handleToAtlasIdx_.emplace(handle, atlases_.size() - 1);

		return handle;
	}

	bool RemoveAtlas(const Handle<T>& handle)
	{
		if (atlases_.empty())
		{
			assert(handleToAtlasIdx_.empty());
			return false;
		}

		auto it = handleToAtlasIdx_.find(handle);
		if (it == handleToAtlasIdx_.end())
		{
			return false;
		}

		size_t oldIdx = it->second;
		size_t backIdx = atlases_.size() - 1;

		const auto& swappedHandle = atlases_[backIdx].GetHandle();
		assert(swappedHandle.IsValid());

		std::swap(atlases_[oldIdx], atlases_[backIdx]);

		atlases_.pop_back();

		handleToAtlasIdx_[swappedHandle] = oldIdx;
		handleToAtlasIdx_.erase(handle);

		return true;
	}

	template <typename LoadData>
	Result<Handle<T>> ReloadAtlas(const Handle<T>& handle, SDL_Renderer* renderer, LoadData&& data)
	{
		auto* atlas = GetAtlas(handle);
		if (!atlas)
		{
			return MAKE_ERROR("Handle does not belong to an atlas in the map");
		}

		auto loadResult = newAtlas.Load(renderer, std::move(data))  v;
		if (!loadResult.Success())
		{
			assert(RemoveAtlas(handle));
		}

		auto handle = loadResult.GetValue();

		handleToAtlasIdx_.erase(handle);

		return handle;
	}

	const T* GetAtlas(const Handle<T>& handle) const
	{
		return GetAtlas(handle);
	}

	SDL_Texture* GetAtlasTexture(const Handle<T>& handle) const
	{
		auto atlas = GetAtlas(handle);

		return (atlas) ? atlas->GetAtlasTexture() : nullptr;
	}

private:
	T* GetAtlas(const Handle<T>& handle)
	{
		auto it = handleToAtlasIdx_.find(handle);
		if (it == handleToAtlasIdx_.end())
		{
			return nullptr;
		}

		assert(it->second < atlases_.size());

		return &atlases_[it->second];
	}

	std::vector<T> atlases_;
	std::unordered_map<Handle<T>, size_t> handleToAtlasIdx_;
};