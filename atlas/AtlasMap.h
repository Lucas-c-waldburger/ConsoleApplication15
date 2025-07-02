#pragma once
#include "Atlas.h"

template <typename T>
class AtlasMap
{
public:
	template <typename LoadData>
	Result<Handle<T>> LoadNewAtlas(SDL_Renderer* renderer, LoadData&& data)
	{
		T newAtlas{};

		TRY(newAtlas.Load(renderer, std::forward<LoadData>(data)), handle);

		atlases_.emplace(handle, std::move(newAtlas));

		return handle;
	}

	template <typename LoadData>
	Result<Handle<T>> ReloadAtlas(const Handle<T>& handle, SDL_Renderer* renderer, LoadData&& data)
	{
		if (!handle.IsValid())
		{
			return MAKE_ERROR("Invalid handle");
		}

		auto it = atlases_.find(handle);
		if (it == atlases_.end())
		{
			return MAKE_ERROR("Handle not found. The atlas it refers to was either "
				"removed, reloaded, or owned by a different AtlasMap");
		}

		TRY(it->second.Load(renderer, std::move(data)), newHandle);

		atlases_.emplace(newHandle, std::move(it->second));
		atlases_.erase(handle);

		return handle;
	}

	bool RemoveAtlas(const Handle<T>& handle)
	{
		if (!handle.IsValid())
		{
			return false;
		}

		return atlases_.erase(handle);
	}

	const T* GetAtlas(const Handle<T>& handle) const
	{
		auto it = atlases_.find(handle);

		return (it != atlases_.end()) ? &it->second : nullptr;
	}

	SDL_Texture* GetAtlasTexture(const Handle<T>& handle) const
	{
		auto it = atlases_.find(handle);

		return (it != atlases_.end()) ? it->second.GetAtlasTexture() : nullptr;
	}

private:
	std::unordered_map<Handle<T>, T> atlases_;
};