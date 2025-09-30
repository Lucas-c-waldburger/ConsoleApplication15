#include "ParticleSystem.h"
#include "../ecs/Ecs.h"


void ParticleSystem::Update(float delta)
{
    auto entities = ECS::GetAllEntitiesWith<
        Transform, Timer, Renderable, ParticleBehavior>();

    for (auto& entity : entities)
    {
        if (!entity.IsValid())
        {
            continue;
        }

        auto [tf, timer, rend, bx] = entity.GetComponents<
            Transform, Timer, Renderable, ParticleBehavior>();

        if (timer.duration != bx.lifetime)
        {
            LOG_ERROR_FMT("Internal inconsistency: entity's timer duration ({}) "
                "does not match particle behavior's lifetime ({})", 
                timer.duration, bx.lifetime);

            continue;
        }

        if ((timer.flags & Timer::Flag::Active) == 0)
        {
            continue;
        }

        //const float t = timer.elapsed / timer.duration;

        //tf.position = bx.position.Evaluate(tf.position, delta, t);
        //tf.scale = bx.scale.Evaluate(tf.scale, delta, t);
        //tf.rotation = bx.rotation.Evaluate(tf.rotation, delta, t);

        //auto& mods = rend.profile.mods;
        //mods.color = bx.color.Evaluate(mods.color, delta, t);
        //mods.alpha = bx.alpha.Evaluate(mods.alpha, delta, t);
    }
}
