#pragma once
#include "../core/Dictionary.h"
#include "../core/Result.h"
#include "../core/commonObjects.h"

class SceneFixture;

using SceneInitializer = Result<Void>(*)(SceneFixture&);

using SceneInitializerMap = std::unordered_map<std::string, SceneInitializer>;

class SceneRegistry
{
public:
	template <typename Fn> requires std::convertible_to<Fn, SceneInitializer>
	bool RegisterScene(std::string&& sceneName, Fn&& init)
	{
		if (scenes_.contains(sceneName))
		{
			return false;
		}

		scenes_.try_emplace(std::move(sceneName), std::forward<Fn>(init));

		return true;
	}

	Result<Void> InitScene(const std::string& sceneName, SceneFixture& fixture)
	{
		auto it = scenes_.find(sceneName);
		if (it == scenes_.end())
		{
			return MAKE_ERROR_FMT("Scene with name '{}' not registered", sceneName);
		}

		if (it->second)
		{
			TRY(std::invoke(it->second, fixture));
		}

		activeScene_ = sceneName;

		return kVoid;
	}

	bool IsSceneRegistered(const std::string& sceneName) const
	{
		return scenes_.contains(sceneName);
	}

	const std::string& GetActiveScene() const { return activeScene_; }

private:
	SceneInitializerMap scenes_;
	std::string activeScene_;
};