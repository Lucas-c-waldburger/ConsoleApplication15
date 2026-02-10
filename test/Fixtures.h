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
#include "../systems/SystemManager2.h"
#include "../atlas/NewTextureRepository.h"
#include "../events/EventBus2.h"

class SceneFixture
{
public:
	using SharedPtr = std::shared_ptr<SceneFixture>;

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
	Result<Void> UpdateEntityStates();
	Result<bool> UpdateSDLInputs();
	Result<Void> UpdatePhysics();
	Result<Void> UpdateCamera();	
	Result<Void> UpdateAudio();
	Result<Void> UpdateRender();
	Result<Void> UpdateUi();
	void LoopEnd();

	// getters
	template <typename T> 
	std::unique_ptr<T>& GetSystem() { return systems_.GetSystem<T>(); }

	template <typename T>
	bool IsSystemInitialized() { return systems_.IsSystemInitialized<T>(); }

	HookManager& GetHooks() { return hooks_; }
	TextureRepository& GetTextureRepository() { return textureRepo_; }
	B2World& GetWorld() { return world_; }
	ScriptManager& GetScripts() { return scripts_; }
	SDL_Renderer* GetRenderer() { return SDLite::Renderer(); }
	SDL_Window* GetWindow() { return SDLite::Window(); }
	
	float GetDeltaTime() const { return systems_.GetSystem<GameLoopSystem>()->GetDeltaTime(); }

	// bundled processes
	Result<Void> RenderScene(SDL_Color bgColor = SDLite::kColorWhite);

	EventBus2& GetEventBus() { return eventBus_; }
	Camera& GetCamera();

	void TearDown();
	
	template <typename...Ts>
	void SetTestScriptFile(std::string_view scriptFileName, std::function<void(Lua&)>&& setupFn);

	// creation
	static Result<std::shared_ptr<SceneFixture>> GetInstance();

	// system scheduling
	template <ImplementsSystemUpdate T, typename...Args>
		requires std::constructible_from<T, Args...>
	T& RegisterSystem(Phase phase, Args&&...args)
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type argument should have no cv-ref qualifiers");

		return userSystemScheduler_.RegisterSystem(
			phase, std::forward<Args>(args)...);
	}

private:
	void UpdateTimers();

	TextureRepository textureRepo_;
	SystemManager systems_;
	HookManager hooks_;
	B2World world_;
	ScriptManager scripts_;
	TestScript testScript_;
	EventBus2 eventBus_;
	UserSystemScheduler userSystemScheduler_;
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

		TRY(UpdateEntityStates());

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
