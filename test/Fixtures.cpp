#include "Fixtures.h"
#include "../ecs/Ecs.h"
#include "../physics/B2World.h"
#include "../gui/GuiContext.h"

SceneFixture::~SceneFixture()
{
	TearDown(); 
}

Result<Void> SceneFixture::RunGameLoopMs(int ms)
{
	assert(ms > 0);
	float sec = static_cast<float>(ms) / 1000.0f;
	float elapsed = 0.0f;

	while (elapsed < sec)
	{
		LoopStart();

		elapsed += GetDeltaTime();

		TRY(UpdateSDLInputs(), cont);
		if (!cont)
		{
			break;
		}

		TRY(UpdateEntityStates());

		TRY(UpdatePhysics());

		if (elapsed >= sec)
		{
			int x = 0;
		}

		TRY(UpdateAudio());

		TRY(UpdateCamera());

		TRY(RenderScene());

		LoopEnd();
	}

	return Void{};
}

Result<Void> SceneFixture::StepGameLoop(int count)
{
	while (true)
	{
		LoopStart();
		if (count-- <= 0)
		{
			break;
		}

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

Result<Void> SceneFixture::RunGameLoop()
{
	while (true)
	{
		LoopStart();

		systems_.RunSystemUpdates(Phase::Setup, GetDeltaTime());

		TRY(UpdateSDLInputs(), cont);
		if (!cont)
		{
			break;
		}

		systems_.RunSystemUpdates(Phase::Input, GetDeltaTime());

		systems_.RunSystemUpdates(Phase::Simulation, GetDeltaTime());
		TRY(UpdatePhysics());
		systems_.RunSystemUpdates(Phase::SimResponse, GetDeltaTime());

		TRY(UpdateAudio());
		TRY(UpdateCamera());

		systems_.RunSystemUpdates(Phase::Presentation, GetDeltaTime());

		TRY(RenderScene(SDLite::kColorBlack));

		LoopEnd();

		systems_.RunSystemUpdates(Phase::Cleanup, GetDeltaTime());
	}

	return Void{};
}

Result<Void> SceneFixture::RunGameLoopConditional(bool& cond)
{
	while (cond)
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

void SceneFixture::LoopStart()
{
	assert(systems_.IsSystemRegistered<GameLoopSystem>());
	systems_.GetSystem<GameLoopSystem>().UpdateLoopStepStart(eventBus_);

	hooks_.SetHookPoint<HookPoint::LoopStart>();

	UpdateTimers();
}

Result<Void> SceneFixture::UpdateEntityStates()
{
	assert(systems_.IsSystemRegistered<EntityStateSystem>());
	assert(systems_.IsSystemRegistered<GameLoopSystem>());

	float delta = systems_.GetSystem<GameLoopSystem>().GetDeltaTime();

	systems_.GetSystem<EntityStateSystem>().Update(delta);

	return Void{};
}

Result<bool> SceneFixture::UpdateSDLInputs()
{
	assert(systems_.IsSystemRegistered<SDLInputSystem>());
	auto& inputSys = systems_.GetSystem<SDLInputSystem>();

	return inputSys.Update(GetDeltaTime(), eventBus_, textureRepo_, GetRenderer());
}

Result<Void> SceneFixture::UpdatePhysics()
{
	assert(systems_.IsSystemRegistered<PhysicsSystem>());
	assert(world_.IsValid());

	static constexpr float kTimeStep = 1.0f / 60.0f;
	systems_.GetSystem<PhysicsSystem>().Update(&world_, eventBus_, kTimeStep, 4);

	return Void{};
}

Result<Void> SceneFixture::UpdateCamera()
{
	assert(systems_.IsSystemRegistered<CameraSystem>());
	assert(systems_.IsSystemRegistered<GameLoopSystem>());
	float delta = systems_.GetSystem<GameLoopSystem>().GetDeltaTime();

	systems_.GetSystem<CameraSystem>().Update(delta);

	return Void{};
}

Result<Void> SceneFixture::UpdateAudio()
{
	assert(systems_.IsSystemRegistered<AudioSystem>());

	systems_.GetSystem<AudioSystem>().Update();

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
	systems_.GetSystem<GameLoopSystem>().UpdateLoopStepRender(eventBus_);
}

Camera& SceneFixture::GetCamera()
{
	assert(systems_.IsSystemRegistered<CameraSystem>());
	return systems_.GetSystem<CameraSystem>().GetCamera();
}

Result<Void> SceneFixture::RenderScene(SDL_Color bgColor)
{
#if IMGUI_ENABLED
	assert(systems_.IsSystemRegistered<GuiSystem>());

	systems_.GetSystem<GuiSystem>().NewFrame();
	systems_.GetSystem<GuiSystem>().Update();
	systems_.GetSystem<GuiSystem>().RenderPrepare();
#endif

	SDLite::Renderer().Clear(bgColor);

	TRY(UpdateRender());

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
	fixture->systems_.RegisterSystem<EntityStateSystem>();
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