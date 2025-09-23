#include "Premades.h"

test::ParticleEmitter::ParticleEmitter(
    EventBus2& bus, Renderable renderable, SDL_FPoint emitterPos, 
    ParticleGenerator&& generator, int particlesPerSec, size_t maxCount) : 
    renderable_(std::move(renderable)), emitter_(ECS::CreateEntity()), 
    maxParticles_(maxCount), particles_(maxCount), particleGenerator_(std::move(generator))
{
        assert(!std::holds_alternative<std::monostate>(renderable_.renderData));
        assert(emitter_.IsValid());

        auto& tf = emitter_.AddComponent<Transform>();
        tf.position = emitterPos; 

        auto& spawnTimer = emitter_.AddComponent<Timer>();
        assert(particlesPerSec > 0);

        spawnTimer.duration = 1.0f / static_cast<float>(particlesPerSec);
        spawnTimer.flags = (Timer::Active | Timer::Repeating);

        auto& tokens = emitter_.AddComponent<SignalTokenStorage>().signalTokens;
        tokens.push_back(bus.ConnectToEvent(GetSpawnerLambda()));
}

test::ParticleEmitter::~ParticleEmitter()
{
    emitter_.Destroy();
    for (auto& [particle, _] : particles_)
    {
        particle.Destroy();
    }
}
