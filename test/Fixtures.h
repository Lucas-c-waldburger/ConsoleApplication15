#pragma once
#include <format>
#include <cstdlib>
#include <cassert>
#include <typeindex>
#include "../physics/B2World.h"
#include "../scripting/ScriptManager.h"
#include "../atlas/AtlasManager.h"
#include "../core/Monitoring.h"
#include "../core/Hooks.h"
#include "../core/Counter.h"
#include "../systems/SystemManager.h"

template <typename... Ts>
inline std::array<std::type_index, sizeof...(Ts)> MakeTypeIndexArray()
{
	return { std::type_index(typeid(Ts))... };
}


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

	//template <typename...Ts> requires (sizeof...(Ts) == TypeList<SYSTEM_REGISTRY>::size)
	/*class SystemUpdateOrder
	{
	public:
		SystemUpdateOrder() : order_(MakeTypeIndexArray<Ts...>()) {}

		template <SomeTypeInPack<SYSTEM_REGISTRY> T>
		Result<Void> MarkUpdated()
		{
			if (nextIndex_ >= order_.size())
			{
				return MAKE_ERROR("LoopStart was not called first!");
			}

			if (order_[nextIndex_] != typeid(T))
			{
				return MAKE_ERROR_FMT("Systems updated out of order! Expected '{}', got '{}'",
					order_[nextIndex_].name(), typeid(T).name());
			}

			++nextIndex_;

			return Void{};
		}

		void Reset() { nextIndex_ = 0; }

	private:
		std::array<std::type_index, TypeList<SYSTEM_REGISTRY>::size> order_;
		size_t nextIndex_ = 0;
	};*/

	/*using SystemOrder = SystemUpdateOrder<
		EventSystem,
		PhysicsSystem,
		CameraSystem,
		RenderSystem
	>;*/

	SceneFixture() = default;
	~SceneFixture();

	// updates
	void LoopStart();
	Result<bool> UpdateSDLInputs();
	Result<Void> UpdatePhysics();
	Result<Void> UpdateCamera();	
	Result<Void> UpdateRender();
	void LoopEnd();

	// getters
	template <typename T> 
	std::unique_ptr<T>& GetSystem() { return systems_.GetSystem<T>(); }
	HookManager& GetHooks() { return hooks_; }
	impl::TextureManager& GetTextures() { return textures_; }
	B2World& GetWorld() { return world_; }
	ScriptManager& GetScripts() { return scripts_; }

	// helpers
	template <AtlasType T>
	Result<Handle<T>> LoadTextureAtlas(AtlasInfo<T> info)
	{
		return textures_.LoadAtlas(SDLite::Renderer(), std::move(info));
	}
	
	template <typename...Ts>
	void SetTestScriptFile(std::string_view scriptFileName, std::function<void(Lua&)>&& setupFn);

	// creation
	static Result<std::shared_ptr<SceneFixture>> GetInstance();

private:
	//SystemOrder systemOrder_;
	impl::TextureManager textures_;
	impl::SystemManager systems_;
	HookManager hooks_;
	B2World world_;
	ScriptManager scripts_;
	Counter counter_;
	TestScript testScript_;
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
