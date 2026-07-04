#include "Fixtures.h"
#include "../ecs/Ecs.h"
#include "../physics/B2World.h"
#include "../gui/GuiContext.h"

SceneFixture::~SceneFixture()
{
	TearDown(); 
}

Result<bool> SceneFixture::RunGameLoopImpl()
{
	LoopStart();

	TRY(UpdateSDLInputs(), cont);
	if (!cont)
	{
		return false;
	}

	TRY(UpdatePhysics());
	TRY(UpdateAudio());
	TRY(UpdateCamera());
	TRY(RenderScene());

	LoopEnd();

	return true;
}

Result<Void> SceneFixture::RunGameLoopMs(int ms)
{
	assert(ms > 0);
	float sec = static_cast<float>(ms) / 1000.0f;
	float elapsed = 0.0f;

	while (elapsed < sec)
	{
		TRY(RunGameLoopImpl(), cont);
		if (!cont)
		{
			break;
		}

		elapsed += GetDeltaTime();
	}

	return Void{};
}

Result<Void> SceneFixture::StepGameLoop(int count)
{
	while (true)
	{
		if (count-- <= 0)
		{
			break;
		}

		TRY(RunGameLoopImpl(), cont);
		if (!cont)
		{
			break;
		}
	}

	return Void{};
}

Result<Void> SceneFixture::RunGameLoop()
{
	while (true)
	{
		TRY(RunGameLoopImpl(), cont);		
		if (!cont)
		{
			break;
		}
	}

	return Void{};
}

Result<Void> SceneFixture::RunGameLoopConditional(bool& cond)
{
	while (cond)
	{
		TRY(RunGameLoopImpl(), cont);	
		if (!cont)
		{
			break;
		}
	}

	return Void{};
}

void SceneFixture::LoopStart()
{
	assert(systems_.IsSystemRegistered<GameLoopSystem>());
	systems_.GetSystem<GameLoopSystem>().UpdateLoopStepStart(eventBus_);

	hooks_.SetHookPoint<HookPoint::LoopStart>();

	UpdateTimers();

	systems_.RunSystemUpdates(Phase::Setup, GetDeltaTime());
}

Result<bool> SceneFixture::UpdateSDLInputs()
{
	assert(systems_.IsSystemRegistered<SDLInputSystem>());
	auto& inputSys = systems_.GetSystem<SDLInputSystem>();

	bool cont = inputSys.Update(GetDeltaTime(), eventBus_, textureRepo_, GetRenderer());
	if (!cont)
	{
		return false;
	}

	systems_.RunSystemUpdates(Phase::Input, GetDeltaTime());

	return true;
}

Result<Void> SceneFixture::UpdatePhysics()
{
	systems_.RunSystemUpdates(Phase::Simulation, GetDeltaTime());

	assert(systems_.IsSystemRegistered<PhysicsSystem>());
	assert(world_.IsValid());

	static constexpr float kTimeStep = 1.0f / 60.0f;
	systems_.GetSystem<PhysicsSystem>().Update(&world_, eventBus_, kTimeStep, 4);

	systems_.RunSystemUpdates(Phase::SimResponse, GetDeltaTime());

	return Void{};
}

Result<Void> SceneFixture::UpdateCamera()
{
	assert(systems_.IsSystemRegistered<CameraSystem>());

	systems_.GetSystem<CameraSystem>().Update(GetDeltaTime());

	return Void{};
}

Result<Void> SceneFixture::UpdateAudio()
{
	assert(systems_.IsSystemRegistered<AudioSystem>());

	systems_.GetSystem<AudioSystem>().Update(GetDeltaTime());

	return Void{};
}

Result<Void> SceneFixture::UpdateRender()
{
	assert(systems_.IsSystemRegistered<NewRenderSystem>());
	assert(systems_.IsSystemRegistered<GameLoopSystem>());
	assert(systems_.IsSystemRegistered<SpriteAnimationSystem>());

	systems_.GetSystem<GameLoopSystem>().UpdateLoopStepRender(eventBus_);

	systems_.GetSystem<SpriteAnimationSystem>().Update(textureRepo_);

	auto& cam = systems_.GetSystem<CameraSystem>().GetCamera();
	systems_.GetSystem<NewRenderSystem>().Update(
		SDLite::Renderer(), cam, textureRepo_
	);

	return Void{};
}

void SceneFixture::UpdateTimers()
{
	assert(systems_.IsSystemRegistered<TimerSystem>());
	assert(systems_.IsSystemRegistered<GameLoopSystem>());
	float delta = systems_.GetSystem<GameLoopSystem>().GetDeltaTime();

	systems_.GetSystem<TimerSystem>().Update(delta, eventBus_);
}

Result<Void> SceneFixture::UpdateUi()
{
	return Void{};
}

void SceneFixture::LoopEnd()
{
	assert(systems_.IsSystemRegistered<GameLoopSystem>());
	systems_.GetSystem<GameLoopSystem>().UpdateLoopStepEnd(eventBus_);

	systems_.RunSystemUpdates(Phase::Cleanup, GetDeltaTime());
}

Camera& SceneFixture::GetCamera()
{
	assert(systems_.IsSystemRegistered<CameraSystem>());
	return systems_.GetSystem<CameraSystem>().GetCamera();
}

AudioBank& SceneFixture::GetAudioBank()
{
	assert(systems_.IsSystemRegistered<AudioSystem>());
	return systems_.GetSystem<AudioSystem>().GetAudioBank();
}

MouseState SceneFixture::GetMouseState() const
{
	assert(systems_.IsSystemRegistered<SDLInputSystem>());
	return systems_.GetSystem<SDLInputSystem>().GetMouseEventHandler().GetMouseState();
}

Result<Void> SceneFixture::SerializeState(SerializationSystem::Filepaths fps)
{
	if (fps.texturesPath.empty())
	{
		TRY_ASSIGN(fps.texturesPath, ResourcePath::Json("persistence/textures.json"));
	}
	if (fps.audioPath.empty())
	{
		TRY_ASSIGN(fps.audioPath, ResourcePath::Json("persistence/audio.json"));
	}
	if (fps.entitiesPath.empty())
	{
		TRY_ASSIGN(fps.entitiesPath, ResourcePath::Json("persistence/entities.json"));
	}

	assert(systems_.IsSystemRegistered<SerializationSystem>());
	assert(systems_.IsSystemRegistered<AudioSystem>());

	systems_.GetSystem<SerializationSystem>().SerializeState(
		fps, textureRepo_, systems_.GetSystem<AudioSystem>().GetAudioBank());

	return kVoid;
}

Result<std::vector<Error>> SceneFixture::DeserializeState(SerializationSystem::Filepaths fps)
{
	if (fps.texturesPath.empty())
	{
		TRY_ASSIGN(fps.texturesPath, ResourcePath::Json("persistence/textures.json"));
	}
	if (fps.audioPath.empty())
	{
		TRY_ASSIGN(fps.audioPath, ResourcePath::Json("persistence/audio.json"));
	}
	if (fps.entitiesPath.empty())
	{
		TRY_ASSIGN(fps.entitiesPath, ResourcePath::Json("persistence/entities.json"));
	}

	assert(systems_.IsSystemRegistered<SerializationSystem>());
	assert(systems_.IsSystemRegistered<SDLInputSystem>());
	assert(systems_.IsSystemRegistered<AudioSystem>());

	auto activeEntities = ECS::GetAllActiveEntities();
	for (auto& entity : activeEntities)
	{
		if (!entity.GetRelations().IsChild())
		{
			entity.Destroy();
		}
	}

	return systems_.GetSystem<SerializationSystem>().DeserializeState(
		fps, world_, textureRepo_, systems_.GetSystem<SDLInputSystem>(),
		systems_.GetSystem<AudioSystem>().GetAudioBank(), GetRenderer()
	);
}

Result<Void> SceneFixture::RenderScene()
{
#if IMGUI_ENABLED
	assert(systems_.IsSystemRegistered<GuiSystem>());

	systems_.GetSystem<GuiSystem>().NewFrame();
	systems_.GetSystem<GuiSystem>().Update();
	systems_.GetSystem<GuiSystem>().RenderPrepare();
#endif

	SDLite::Renderer().Clear(config_.screenColor);

	TRY(UpdateRender());

	systems_.RunSystemUpdates(Phase::Presentation, GetDeltaTime());

#if IMGUI_ENABLED
	assert(systems_.IsSystemRegistered<GuiSystem>());

	systems_.GetSystem<GuiSystem>().RenderPresent(GetRenderer());
#endif

	SDLite::Renderer().Show();

	return Void{};
}

void SceneFixture::TearDown()
{
	auto activeEntities = ECS::GetAllActiveEntities();
	for (auto& entity : activeEntities)
	{
		if (!entity.GetRelations().IsChild())
		{
			entity.Destroy();
		}
	}

	world_.Destroy();

#if IMGUI_ENABLED
	assert(GuiContext::IsInitialized());
	GuiContext::Exit();
#endif

	SDLite::Exit();
	Logger::EndSession();
}

Result<std::shared_ptr<SceneFixture>> SceneFixture::GetInstance()
{
	Logger::StartSession();
	SDLite::Start();

	auto fixture = std::make_shared<SceneFixture>();

	fixture->world_ = B2World::Create(0, 9.8f);

#if IMGUI_ENABLED
	TRY(GuiContext::Init(fixture->GetWindow(), fixture->GetRenderer()));
	fixture->systems_.RegisterSystem<GuiSystem>();
#endif 

	fixture->systems_.RegisterSystem<PhysicsSystem>();
	fixture->systems_.RegisterSystem<SDLInputSystem>();
	fixture->systems_.RegisterSystem<TimerSystem>();
	fixture->systems_.RegisterSystem<GameLoopSystem>();
	fixture->systems_.RegisterSystem<AudioSystem>();
	fixture->systems_.RegisterSystem<NewRenderSystem>();
	fixture->systems_.RegisterSystem<SerializationSystem>();
	fixture->systems_.RegisterSystem<SpriteAnimationSystem>();

	Dimensions<float> cameraVp = SDLite::Window().GetSize<float>();
	fixture->systems_.RegisterSystem<CameraSystem>(cameraVp);

	fixture->GetCamera().SetPosition(SDLite::Window().GetLocalCenter<SDL_FPoint>());

	return Result<std::shared_ptr<SceneFixture>>{ std::move(fixture) };
}