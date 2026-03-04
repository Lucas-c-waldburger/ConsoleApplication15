//#include "GirlStateHandlers.h"
//#include "../../../ecs/Ecs.h"
//#include "../../../events/EventBus2.h"
//#include <cassert>
//
//namespace test {
//
//
//GirlStateHandlers::GirlStateHandlers(EventBus2& bus) : eventBus_(&bus)
//{}
//
//void GirlStateHandlers::SetState(State newState)
//{
//	assert(SizedEnumValueInRange(newState));
//
//	auto& active = GetActiveStateHandler();
//	if (!active)
//	{
//		SetFallbackState();
//	}
//
//	active->OnExit(ctx_);
//
//	const auto lastState = currentState_;
//	currentState_ = newState;
//
//	auto& newActive = GetActiveStateHandler();
//	if (!newActive)
//	{
//		SetFallbackState();
//	}
//
//	newActive->OnEnter(ctx_);
//
//	ctx_.girl.GetComponent<GirlState>().animation = currentState_;
//
//	if (eventBus_)
//	{
//		eventBus_->PushAndDispatchEvents(GirlStateChangeEvent{
//			.lastState = lastState,
//			.newState = newState
//		});
//	}
//}
//
//std::unique_ptr<BaseGirlStateHandler>& GirlStateHandlers::GetActiveStateHandler()
//{
//	assert(SizedEnumValueInRange(currentState_));
//
//	return handlers_[static_cast<size_t>(currentState_)];
//}
//
//void GirlStateHandlers::Upkeep(Entity& girl, float dt)
//{
//	assert(girl.IsValid());
//	ctx_.girl = girl;
//
//	auto [tf, targets] =
//		ctx_.girl.GetComponents<Transform, MoveTargets>();
//
//	ctx_.distanceInCurrentAnimFrame.x +=
//		std::abs(tf.position.x - ctx_.lastPosition.x);
//	ctx_.distanceInCurrentAnimFrame.y +=
//		std::abs(tf.position.y - ctx_.lastPosition.y);
//
//	ctx_.timeInCurrentAnimFrame += dt;
//	ctx_.dt = dt;
//	targets.dt = dt;
//	
//	for (auto& handler : handlers_)
//	{
//		if (handler)
//		{
//			handler->Upkeep(ctx_);
//		}
//	}
//}
//
//void GirlStateHandlers::UpdateAnimation()
//{
//	auto& active = GetActiveStateHandler();
//
//	if (active)
//	{
//		active->UpdateAnimation(ctx_);
//	}
//}
//
//void GirlStateHandlers::Cleanup()
//{
//	if (ctx_.girl.HasComponent<NeedsAnimationUpdate>())
//	{
//		ctx_.timeInCurrentAnimFrame = 0.0f;
//		ctx_.distanceInCurrentAnimFrame = { 0.0f, 0.0f };
//	}
//
//	ctx_.lastPosition = ctx_.girl.GetComponent<Transform>().position;
//}
//
//void GirlStateHandlers::SetFallbackState()
//{
//	assert(handlers_[static_cast<size_t>(State::Idle)]);
//
//	SetState(State::Idle);
//}
//
//} // test