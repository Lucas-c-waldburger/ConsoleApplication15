#include "GirlStateCoordinator.h"
#include "../../../systems/SystemManager.h"

namespace test {

// GirlStateCoordinator systems
void GirlStateCoordinator::EvaluationSystem::UpdateImpl(Entity& girl, float dt)
{
	parentCoordinator_->UpdateStateContext(girl, dt);
	parentCoordinator_->stateMachine_.Evaluate(girl);

	girl.GetComponent<GirlState>().animation = parentCoordinator_->stateMachine_.current;
}

void GirlStateCoordinator::IntentSystem::UpdateImpl(Entity& girl, float)
{
	parentCoordinator_->stateMachine_.UpdateIntents(girl);
}

void GirlStateCoordinator::AnimationSystem::UpdateImpl(Entity& girl, float dt)
{
	parentCoordinator_->stateMachine_.UpdateAnimations(girl);
	UpdateSpriteFacingSide(girl, parentCoordinator_->stateMachine_.context.deltas.lastPosition);
	parentCoordinator_->ResolveStateContext(girl);
}

void GirlStateCoordinator::AnimationSystem::UpdateSpriteFacingSide(Entity& girl, SDL_FPoint lastPos)
{
	auto [tf, rend] = girl.GetComponents<Transform, SpriteRenderableComponent>();

	float xMoveDelta = lastPos.x - tf.position.x;

	rend.profile.flip = (tf.position.x < lastPos.x) ? SDL_FLIP_HORIZONTAL :
						(tf.position.x > lastPos.x) ? SDL_FLIP_NONE :
													  rend.profile.flip;
}

// GirlStateCoordinator
GirlStateCoordinator::GirlStateCoordinator(Entity_t girlId) : girlId_(girlId)
{
	stateMachine_[Jumping] = kGirlJumpStateNode;
	stateMachine_[Falling] = kGirlFallStateNode;
	stateMachine_[Idle] = kGirlIdleStateNode;
	stateMachine_[Landing] = kGirlLandStateNode;
	stateMachine_[Walking] = kGirlWalkStateNode;

	auto girl = ECS::GetEntityByID(girlId);
	assert(girl.IsValid());

	auto& sm = stateMachine_;

	const auto& tf = girl.GetComponent<Transform>();

	sm.context.deltas.lastPosition = tf.position;
	
	sm.current = Idle;
	sm[Idle].onEnter(stateMachine_.context, girl);
}

void GirlStateCoordinator::RegisterSystems(SystemManager& sysManager)
{
	sysManager.RegisterSystem<EvaluationSystem>(Phase::Input, *this);
	sysManager.RegisterSystem<IntentSystem>(Phase::Simulation, *this);
	sysManager.RegisterSystem<AnimationSystem>(Phase::Presentation, *this);
}

void GirlStateCoordinator::UpdateStateContext(Entity& e, float dt)
{
	const auto& tf = e.GetComponent<Transform>();

	auto& ctxDeltas = stateMachine_.context.deltas;

	ctxDeltas.deltaAnimMove.x += std::abs(tf.position.x - ctxDeltas.lastPosition.x);
	ctxDeltas.deltaAnimMove.y += std::abs(tf.position.y - ctxDeltas.lastPosition.y);

	ctxDeltas.deltaAnimTime += dt;
	ctxDeltas.deltaFrameTime = dt;
	
	stateMachine_.context.targets.dt = dt;
}

void GirlStateCoordinator::ResolveStateContext(Entity& e)
{
	auto& ctx = stateMachine_.context;
	auto& deltas = ctx.deltas;

	if (e.HasComponent<NeedsAnimationUpdate>())
	{
		deltas.deltaAnimMove = { 0.0f, 0.0f };
		deltas.deltaAnimTime = 0.0f;
	}

	auto [tf, intent] = e.GetComponents<Transform, GirlIntent>();

	ctx.deltas.lastPosition = tf.position;

	intent.attackIntent = InputState::None;
	intent.jumpIntent = InputState::None;
	intent.moveIntent = { 0.0f, 0.0f };

	ctx.flags = 0;
}

} // test