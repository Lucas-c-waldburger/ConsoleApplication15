#include "Premades.h"

test::ParticleEmitter::~ParticleEmitter()
{
    emitter_.Destroy();
    for (auto& [particle, _] : particles_)
    {
        particle.Destroy();
    }
}

void test::ParticleEmitter::AddParticle()
{
    if (liveCount_ >= maxParticles_)
    {
        return;
    }

    if (!bxGenerator_)
    {
        LOG_ERROR("No particle behavior generator set");
        return;
    }

    auto& [particle, bx] = particles_[liveCount_++];

    if (!particle.IsValid())
    {
        particle = ECS::CreateEntity();

        // stop them from firing when destroyed or clogging up timer events
        particle.SetEventProduction<events::TimerFired>(false);
        particle.SetEventProduction<events::EntityDestroyed>(false);
    }

    particle.SetComponentVisibility(true);

    bx = bxGenerator_();
    assert(bx.lifetime > 0.0f);

    const SDL_FPoint emitterPos = emitter_.GetComponent<Transform>().position;
    particle.AddComponent<Transform>().position = emitterPos + bx.offset;

    auto& srcRenderData = emitter_.GetComponent<Renderable>().renderData;
    auto& renderable = particle.AddComponent<Renderable>();
    renderable.renderData = srcRenderData;
    renderable.profile.debugDraw.boundingBox.on = false;

    auto& timer = particle.AddComponent<Timer>();
    timer.duration = bx.lifetime;
    timer.flags = Timer::Flag::Active;
}

void test::ParticleEmitter::Update(float delta)
{
    auto& spawnTimer = emitter_.GetComponent<Timer>();
    if ((spawnTimer.flags & Timer::Flag::Active) == 0)
    {
        AddParticle();
        spawnTimer.flags = Timer::Flag::Active;
    }

    if (liveCount_ <= 0)
    {
        return;
    }

    auto kill = [this](Entity& particle, size_t& i) {
        // wont be processed by any system, will be considered IsValid()
        particle.SetComponentVisibility(false);

        if (--liveCount_ <= i)
        {
            return;
        }

        std::swap(particles_[i], particles_[liveCount_]);
        --i;
    };

    auto emitterPos = emitter_.GetComponent<Transform>().position;

    for (size_t i = 0; i < liveCount_; i++)
    {
        bool particleValid = UpdateParticle(particles_[i], delta);
        if (!particleValid)
        {
            kill(particles_[i].first, i);
        }
    }
}

bool test::ParticleEmitter::UpdateParticle(std::pair<Entity, ParticleBehavior>& particleBx,
                                           float delta)
{
    auto& [particle, bx] = particleBx;

    if (!particle.IsValid())
    {
        return false;
    }

    auto [tf, timer, rend] = particle.GetComponents<Transform, Timer, Renderable>();

    if ((timer.flags & Timer::Flag::Active) == 0)
    {
        return false;
    }

    const float t = timer.elapsed / timer.duration;

    tf.position = bx.position.Evaluate(tf.position, delta, t);
    tf.scale = bx.scale.Evaluate(tf.scale, delta, t);
    tf.rotation = bx.rotation.Evaluate(tf.rotation, delta, t);

    auto& mods = rend.profile.mods;
    mods.color = bx.color.Evaluate(mods.color, delta, t);
    mods.alpha = bx.alpha.Evaluate(mods.alpha, delta, t);

    LOG_DEBUG_FMT("R:{} G:{} B:{} A:{}", 
        mods.color.r, mods.color.g, mods.color.b, mods.alpha);

    return true;
}