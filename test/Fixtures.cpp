#include "Fixtures.h"
#include "../ecs/Ecs.h"

SceneFixture::~SceneFixture()
{
	world_.Destroy();

	Logger::EndSession();
	SDLite::Exit();
}

void SceneFixture::LoopStart()
{
	counter_.Update();

	//systemOrder_.Reset();

	hooks_.SetHookPoint<HookPoint::LoopStart>();
}

Result<bool> SceneFixture::UpdateSDLInputs()
{
	//TRY(systemOrder_.MarkUpdated<EventSystem>());

	assert(systems_.IsSystemInitialized<SDLInputSystem>());
	auto& inputSys = systems_.GetSystem<SDLInputSystem>();

	return inputSys->Update();
}

Result<Void> SceneFixture::UpdatePhysics()
{
	//TRY(systemOrder_.MarkUpdated<PhysicsSystem>());

	assert(systems_.IsSystemInitialized<PhysicsSystem>());
	assert(world_.IsValid());

	systems_.GetSystem<PhysicsSystem>()->Update(&world_, 1.0f / 60.0f, 4);

	world_.Step(1.0f / 60.0f, 4);

	return Void{};
}

Result<Void> SceneFixture::UpdateCamera()
{
	//TRY(systemOrder_.MarkUpdated<CameraSystem>());

	assert(systems_.IsSystemInitialized<CameraSystem>());

	systems_.GetSystem<CameraSystem>()->Update(counter_.GetDelta());

	return Void{};
}

Result<Void> SceneFixture::UpdateRender()
{
	//TRY(systemOrder_.MarkUpdated<RenderSystem>());

	assert(systems_.IsSystemInitialized<RenderSystem>());

	auto& cam = systems_.GetSystem<CameraSystem>()->GetCamera();

	systems_.GetSystem<RenderSystem>()->Update(SDLite::Renderer(), cam, textures_);

	return Void{};
}

Result<std::shared_ptr<SceneFixture>> SceneFixture::GetInstance()
{
	Logger::StartSession();
	SDLite::Start();
	TRY((RegisterCustomEventDataTypes<CUSTOM_EVENT_DATA_REGISTRY>()));

	auto fixture = std::make_shared<SceneFixture>();

	fixture->world_ = B2World::Create(0, 9.8f);

	fixture->systems_.InitializeSystem<RenderSystem>();
	fixture->systems_.InitializeSystem<PhysicsSystem>();
	fixture->systems_.InitializeSystem<SDLInputSystem>();

	Dimensions<float> cameraVp = { static_cast<float>(SDLite::kWindowWidth),
								   static_cast<float>(SDLite::kWindowHeight) };

	auto& camSystem = fixture->systems_.InitializeSystem<CameraSystem>(cameraVp);

	static constexpr SDL_FPoint screenCenter = {
		static_cast<float>(SDLite::kWindowWidth) / 2.0f,
		static_cast<float>(SDLite::kWindowHeight) / 2.0f
	};

	camSystem->GetCamera().SetPosition(screenCenter);

	assert(fixture->systems_.AllSystemsInitialized());

	return Result<std::shared_ptr<SceneFixture>>{ std::move(fixture) };
}


