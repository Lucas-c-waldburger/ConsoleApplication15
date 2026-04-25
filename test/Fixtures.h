#pragma once
#include <format>
#include <cstdlib>
#include <cassert>
#include <typeindex>
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

	enum FixtureFlag : uint8_t
	{
		ImGuiEnabled = 1 << 0
	};

	struct SceneConfiguration
	{
		SDL_Color screenColor = SDLite::kColorBlack;
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

	// getters
	SystemManager& GetSystemManager() { return systems_; }
	const SystemManager& GetSystemManager() const { return systems_; }

	template <typename T>
	T& GetSystem() { return systems_.GetSystem<T>(); }
	template <typename T> 
	const T& GetSystem() const { return systems_.GetSystem<T>(); }

	template <typename T>
	bool IsSystemRegistered() { return systems_.IsSystemRegistered<T>(); }

	HookManager& GetHooks() { return hooks_; }
	TextureRepository& GetTextureRepository() { return textureRepo_; }
	B2World& GetWorld() { return world_; }
	ScriptManager& GetScripts() { return scripts_; }
	SDL_Renderer* GetRenderer() { return SDLite::Renderer(); }
	SDL_Window* GetWindow() { return SDLite::Window(); }
	SceneConfiguration& GetConfiguration() { return config_; }
	EventBus& GetEventBus() { return eventBus_; }
	Camera& GetCamera();

	float GetDeltaTime() const { return systems_.GetSystem<GameLoopSystem>().GetDeltaTime(); }
	
	template <typename...Ts>
	void SetTestScriptFile(std::string_view scriptFileName, std::function<void(Lua&)>&& setupFn);

	// create/destroy
	static Result<std::shared_ptr<SceneFixture>> GetInstance();
	void TearDown();

	// system scheduling
	template <typename T, typename...Args>
	T& RegisterSystem(Args&&...args)
	{
		return systems_.RegisterSystem<T>(std::forward<Args>(args)...);
	}

private:
	void UpdateTimers();
	Result<Void> RenderScene();

	Result<bool> RunGameLoopImpl();

	TextureRepository textureRepo_;
	SystemManager systems_;
	HookManager hooks_;
	B2World world_;
	ScriptManager scripts_;
	TestScript testScript_;
	EventBus eventBus_;
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
