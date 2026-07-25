#pragma once
#include <format>
#include <cstdlib>
#include <cassert>
#include <typeindex>
#include <deque>
#include "SceneRegistry.h"
#include "../physics/B2World.h"
#include "../scripting/ScriptManager.h"
#include "../core/Monitoring.h"
#include "../core/Hooks.h"
#include "../core/Counter.h"
#include "../systems/SystemManager.h"
#include "../atlas/NewTextureRepository.h"
#include "../events/EventBus2.h"

class SceneFixture
{
public:
	using SharedPtr = std::shared_ptr<SceneFixture>;
	using WeakPtr = std::weak_ptr<SceneFixture>;

	static constexpr std::string_view kScriptResourcesPathFmt = 
		R"(C:\Users\Lucas\source\repos\ConsoleApplication15\resources\scripts\{})";

	struct TestScript
	{
		Lua lua;
		FileChangeMonitor fileMonitor;
		Handle<HookAttachment> hookAttachmentHandle;
	};

	struct SceneConfiguration
	{
		SDL_Color screenColor = SDLite::kColorBlack;
		SDL_FPoint worldGravity = { 0, 9.8f };
	};
	   
	SceneFixture() = default;
	~SceneFixture();

	// main loop
	Result<Void> RunGameLoop();
	Result<Void> StepGameLoop(int count);
	Result<Void> RunGameLoopMs(int ms);
	template <typename Fn> requires std::is_invocable_r_v<bool, Fn>
	Result<Void> RunGameLoopCondition(Fn&& fn);
	Result<Void> RunGameLoopConditional(bool& cond);

	// updates
	void LoopStart();
	Result<bool> UpdateSDLInputs();
	Result<Void> UpdatePhysics();
	Result<Void> UpdateCamera();	
	Result<Void> UpdateAudio();
	Result<Void> UpdateRender();
	Result<Void> UpdateUi();
	void LoopEnd();

	// systems
	SystemManager& GetSystemManager() { return systems_; }
	const SystemManager& GetSystemManager() const { return systems_; }

	template <typename T>
	T& GetSystem() { return systems_.GetSystem<T>(); }
	template <typename T> 
	const T& GetSystem() const { return systems_.GetSystem<T>(); }

	template <typename T>
	bool IsSystemRegistered() { return systems_.IsSystemRegistered<T>(); }

	template <typename T, typename...Args>
	T& RegisterSystem(Args&&...args)
	{
		return systems_.RegisterSystem<T>(std::forward<Args>(args)...);
	}

	// scenes
	template <typename Fn> requires std::convertible_to<Fn, SceneInitializer>
	bool RegisterScene(std::string sceneName, Fn&& init)
	{
		return sceneRegistry_.RegisterScene(std::move(sceneName), std::forward<Fn>(init));
	}

	Result<Void> LoadScene(const std::string& sceneName, const SceneConfiguration& config = {})
	{
		if (!sceneRegistry_.IsSceneRegistered(sceneName))
		{
			return MAKE_ERROR_FMT("Scene with name '{}' not registered", sceneName);
		}

		ResetForNewScene(config);

		return sceneRegistry_.InitScene(sceneName, *this);
	}

	bool IsSceneRegistered(const std::string& sceneName) const
	{
		return sceneRegistry_.IsSceneRegistered(sceneName);
	}

	const std::string& GetActiveScene() const { return sceneRegistry_.GetActiveScene(); }

	HookManager& GetHooks() { return hooks_; }
	TextureRepository& GetTextureRepository() { return textureRepo_; }
	B2World& GetWorld() { return world_; }
	ScriptManager& GetScripts() { return scripts_; }
	SDL_Renderer* GetRenderer() { return SDLite::Renderer(); }
	SDL_Window* GetWindow() { return SDLite::Window(); }
	SceneConfiguration& GetConfiguration() { return config_; }
	EventBus& GetEventBus() { return eventBus_; }
	Camera& GetCamera();
	AudioBank& GetAudioBank();
	MouseState GetMouseState() const;

	Result<Void> SerializeState(SerializationSystem::Filepaths fps = {});
	Result<std::vector<Error>> DeserializeState(SerializationSystem::Filepaths fps = {});

	void SerializeStateToJson(nlohmann::json& j) const;
	std::vector<Error> DeserializeStateFromJson(const nlohmann::json& j);

	float GetDeltaTime() const { return systems_.GetSystem<GameLoopSystem>().GetDeltaTime(); }
	
	template <typename...Ts>
	void SetTestScriptFile(std::string_view scriptFileName, std::function<void(Lua&)>&& setupFn);

	// create/destroy
	static Result<std::shared_ptr<SceneFixture>> GetInstance(const SceneConfiguration& config = {});

	void TearDown();

	void ResetForNewScene(const SceneConfiguration& config);

private:
	void UpdateTimers();
	Result<Void> RenderScene();

	Result<bool> RunGameLoopImpl();

	void SerializeSceneToJson(nlohmann::json& j) const;
	Result<Void> DeserializeSceneFromJson(const nlohmann::json& j);

	TextureRepository textureRepo_;
	SystemManager systems_;
	HookManager hooks_;
	B2World world_;
	ScriptManager scripts_;
	TestScript testScript_;
	EventBus eventBus_;
	SceneRegistry sceneRegistry_;
	SceneConfiguration config_;
};

template<typename Fn> requires std::is_invocable_r_v<bool, Fn>
inline Result<Void> SceneFixture::RunGameLoopCondition(Fn&& fn)
{
	while (fn())
	{
		LoopStart();

		TRY(UpdateSDLInputs(), cont);
		if (!cont)
		{
			break;
		}

		TRY(UpdatePhysics());

		TRY(UpdateAudio());

		TRY(UpdateCamera());

		TRY(RenderScene());

		LoopEnd();
	}

	return Void{};
}

template<typename...Ts>
inline void SceneFixture::SetTestScriptFile(std::string_view scriptFileName, std::function<void(Lua&)>&& setupFn)
{
	if (testScript_.hookAttachmentHandle.IsValid())
	{
		hooks_.Detach(testScript_.hookAttachmentHandle);
	}

	std::string scriptPath = std::format(kScriptResourcesPathFmt, scriptFileName);

	testScript_.fileMonitor.SetFilePath(scriptPath);

	testScript_.lua = Lua::GetInstance<Ts...>();
	testScript_.lua.SetScriptInfo({
		.scriptType = ScriptType::File,
		.path = std::move(scriptPath)
	});

	if (setupFn)
	{
		setupFn(testScript_.lua);
	}

	testScript_.hookAttachmentHandle = hooks_.Attach(
		HookPoint::LoopStart, [this]() 
		{
			if (testScript_.fileMonitor.FileDidChange())
			{
				LOG_IF_ERROR(testScript_.lua.Run());
			}

			return ReturnSignal::KeepObserving;
		});
	
	assert(testScript_.hookAttachmentHandle.IsValid());
}
