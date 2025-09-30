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
#include "../atlas/TextureRepository.h"
#include "../events/EventBus2.h"

class SceneFixture
{
public:
	static constexpr std::string_view kScriptResourcesPathFmt = 
		R"(C:\Users\Lucas\source\repos\ConsoleApplication15\resources\scripts\{})";

	struct TestScript
	{
		Lua lua;
		FileChangeMonitor fileMonitor;
		Handle<HookAttachment> hookAttachmentHandle;
	};

	SceneFixture() = default;
	~SceneFixture();

	// updates
	void LoopStart();
	Result<Void> UpdateEntityStates();
	Result<bool> UpdateSDLInputs();
	Result<Void> UpdatePhysics();
	Result<Void> UpdateCamera();	
	Result<Void> UpdateRender();
	void LoopEnd();

	// getters
	template <typename T> 
	std::unique_ptr<T>& GetSystem() { return systems_.GetSystem<T>(); }
	HookManager& GetHooks() { return hooks_; }
	TextureRepository& GetTextureRepository() { return textureRepo_; }
	B2World& GetWorld() { return world_; }
	ScriptManager& GetScripts() { return scripts_; }


	// helpers
	template <SupportedAtlasType T, typename LoadData>
	Result<Handle<T>> LoadTextureAtlas(LoadData&& loadData)
	{
		return textureRepo_.LoadNewAtlas<T>(SDLite::Renderer(), std::forward<LoadData>(loadData));
	}
	Result<Handle<SpriteSeriesAtlas>> LoadNewSpriteSeriesAtlas(SpriteSeriesResourcePackets&& packets)
	{
		return textureRepo_.LoadNewAtlas<SpriteSeriesAtlas>(SDLite::Renderer(), std::move(packets));
	}
	Result<Handle<GlyphAtlas>> LoadNewGlyphAtlas(FontResourcePacket& packet)
	{
		return textureRepo_.LoadNewAtlas<GlyphAtlas>(SDLite::Renderer(), std::move(packet));
	}
	float GetDeltaTime() const { return systems_.GetSystem<GameLoopSystem>()->GetDeltaTime(); }

	// bundled processes
	Result<Void> RenderScene(SDL_Color bgColor = SDLite::kColorWhite);

	EventBus2& GetEventBus() { return eventBus_; }

	void TearDown();
	
	template <typename...Ts>
	void SetTestScriptFile(std::string_view scriptFileName, std::function<void(Lua&)>&& setupFn);

	// creation
	static Result<std::shared_ptr<SceneFixture>> GetInstance();

private:
	void UpdateTimers();

	TextureRepository textureRepo_;
	impl::SystemManager systems_;
	HookManager hooks_;
	B2World world_;
	ScriptManager scripts_;
	TestScript testScript_;
	EventBus2 eventBus_;
};

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
