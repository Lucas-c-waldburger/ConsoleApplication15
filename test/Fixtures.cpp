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

		TRY(UpdateEntityStates());

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
	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	systems_.GetSystem<GameLoopSystem>()->UpdateLoopStepStart(eventBus_);

	hooks_.SetHookPoint<HookPoint::LoopStart>();

	UpdateTimers();
}

Result<Void> SceneFixture::UpdateEntityStates()
{
	assert(systems_.IsSystemInitialized<EntityStateSystem>());
	assert(systems_.IsSystemInitialized<GameLoopSystem>());

	float delta = systems_.GetSystem<GameLoopSystem>()->GetDeltaTime();

	systems_.GetSystem<EntityStateSystem>()->Update(delta);

	return Void{};
}

Result<bool> SceneFixture::UpdateSDLInputs()
{
	assert(systems_.IsSystemInitialized<SDLInputSystem>());
	auto& inputSys = systems_.GetSystem<SDLInputSystem>();

	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	float delta = systems_.GetSystem<GameLoopSystem>()->GetDeltaTime();

	return inputSys->Update(delta, eventBus_);
}

Result<Void> SceneFixture::UpdatePhysics()
{
	assert(systems_.IsSystemInitialized<PhysicsSystem>());
	assert(world_.IsValid());

	systems_.GetSystem<PhysicsSystem>()->Update(&world_, eventBus_, 1.0f / 60.0f, 4);

	return Void{};
}

Result<Void> SceneFixture::UpdateCamera()
{
	assert(systems_.IsSystemInitialized<CameraSystem>());
	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	float delta = systems_.GetSystem<GameLoopSystem>()->GetDeltaTime();

	systems_.GetSystem<CameraSystem>()->Update(delta);

	return Void{};
}

Result<Void> SceneFixture::UpdateAudio()
{
	assert(systems_.IsSystemInitialized<AudioSystem>());

	systems_.GetSystem<AudioSystem>()->Update();

	return Void{};
}

Result<Void> SceneFixture::UpdateRender()
{
	assert(systems_.IsSystemInitialized<NewRenderSystem>());
	assert(systems_.IsSystemInitialized<GameLoopSystem>());

	systems_.GetSystem<GameLoopSystem>()->UpdateLoopStepRender(eventBus_);

	auto& cam = systems_.GetSystem<CameraSystem>()->GetCamera();
	systems_.GetSystem<NewRenderSystem>()->Update(SDLite::Renderer(), cam, textureRepo_);

	return Void{};
}

void SceneFixture::UpdateTimers()
{
	assert(systems_.IsSystemInitialized<TimerSystem>());
	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	float delta = systems_.GetSystem<GameLoopSystem>()->GetDeltaTime();

	systems_.GetSystem<TimerSystem>()->Update(delta, eventBus_);
}

void SceneFixture::LoopEnd()
{
	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	systems_.GetSystem<GameLoopSystem>()->UpdateLoopStepRender(eventBus_);
}

Result<Void> SceneFixture::RenderScene(SDL_Color bgColor)
{
#if IMGUI_ENABLED
	assert(systems_.IsSystemInitialized<GuiSystem>());

	systems_.GetSystem<GuiSystem>()->NewFrame();
	systems_.GetSystem<GuiSystem>()->Update();
	systems_.GetSystem<GuiSystem>()->RenderPrepare();
#endif

	SDLite::Renderer().Clear(bgColor);

	TRY(UpdateRender());

#if IMGUI_ENABLED
	assert(systems_.IsSystemInitialized<GuiSystem>());

	systems_.GetSystem<GuiSystem>()->RenderPresent(GetRenderer());
#endif

	SDLite::Renderer().Show();

	return Void{};
}

void SceneFixture::TearDown()
{
	auto activeEntities = ECS::GetAllActiveEntities();
	for (auto& entity : activeEntities)
	{
		entity.Destroy();
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
	fixture->systems_.InitializeSystem<GuiSystem>();
#endif 

	fixture->systems_.InitializeSystem<PhysicsSystem>();
	fixture->systems_.InitializeSystem<SDLInputSystem>();
	fixture->systems_.InitializeSystem<TimerSystem>();
	fixture->systems_.InitializeSystem<EntityStateSystem>();
	fixture->systems_.InitializeSystem<GameLoopSystem>();
	fixture->systems_.InitializeSystem<AudioSystem>();
	fixture->systems_.InitializeSystem<NewRenderSystem>();
	fixture->systems_.InitializeSystem<SerializationSystem>();

	Dimensions<float> cameraVp = { static_cast<float>(SDLite::kWindowWidth),
								   static_cast<float>(SDLite::kWindowHeight) };

	auto& cameraSystem = fixture->systems_.InitializeSystem<CameraSystem>(cameraVp);

	static constexpr SDL_FPoint screenCenter = {
		static_cast<float>(SDLite::kWindowWidth) / 2.0f,
		static_cast<float>(SDLite::kWindowHeight) / 2.0f
	};

	cameraSystem->GetCamera().SetPosition(screenCenter);

	return Result<std::shared_ptr<SceneFixture>>{ std::move(fixture) };
}


