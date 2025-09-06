#include "Fixtures.h"
#include "../ecs/Ecs.h"
#include "../physics/B2World.h"

SceneFixture::~SceneFixture()
{
	world_.Destroy();

	Logger::EndSession();
	SDLite::Exit();
}

void SceneFixture::LoopStart()
{
	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	systems_.GetSystem<GameLoopSystem>()->UpdateLoopStepStart();

	hooks_.SetHookPoint<HookPoint::LoopStart>();

	UpdateTimers();
}

Result<bool> SceneFixture::UpdateSDLInputs()
{
	assert(systems_.IsSystemInitialized<SDLInputSystem>());
	auto& inputSys = systems_.GetSystem<SDLInputSystem>();

	return inputSys->Update();
}

Result<Void> SceneFixture::UpdatePhysics()
{
	assert(systems_.IsSystemInitialized<PhysicsSystem>());
	assert(world_.IsValid());

	systems_.GetSystem<PhysicsSystem>()->Update(&world_, 1.0f / 60.0f, 4);

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

Result<Void> SceneFixture::UpdateRender()
{
	assert(systems_.IsSystemInitialized<RenderSystem>());
	assert(systems_.IsSystemInitialized<SpriteAnimationSystem>());
	assert(systems_.IsSystemInitialized<GameLoopSystem>());

	systems_.GetSystem<SpriteAnimationSystem>()->Update(textureRepo_);

	systems_.GetSystem<GameLoopSystem>()->UpdateLoopStepRender();

	auto& cam = systems_.GetSystem<CameraSystem>()->GetCamera();
	systems_.GetSystem<RenderSystem>()->Update(SDLite::Renderer(), cam, textureRepo_);

	return Void{};
}

void SceneFixture::UpdateTimers()
{
	assert(systems_.IsSystemInitialized<TimerSystem>());
	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	float delta = systems_.GetSystem<GameLoopSystem>()->GetDeltaTime();

	systems_.GetSystem<TimerSystem>()->Update(delta);
}

void SceneFixture::LoopEnd()
{
	assert(systems_.IsSystemInitialized<GameLoopSystem>());
	systems_.GetSystem<GameLoopSystem>()->UpdateLoopStepRender();
}

Result<std::shared_ptr<SceneFixture>> SceneFixture::GetInstance()
{
	Logger::StartSession();
	SDLite::Start();

	auto fixture = std::make_shared<SceneFixture>();

	fixture->world_ = B2World::Create(0, 9.8f);

	fixture->systems_.InitializeSystem<RenderSystem>();
	fixture->systems_.InitializeSystem<SpriteAnimationSystem>();
	fixture->systems_.InitializeSystem<PhysicsSystem>();
	fixture->systems_.InitializeSystem<SDLInputSystem>();
	fixture->systems_.InitializeSystem<TimerSystem>();
	fixture->systems_.InitializeSystem<EntityStateSystem>();
	fixture->systems_.InitializeSystem<GameLoopSystem>();

	auto& callbackSystem = fixture->systems_.InitializeSystem<EventCallbackSystem>();
	callbackSystem->ConnectToEventBus();

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


