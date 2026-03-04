//#include "GirlStateHandlerImpls.h"
//#include "GirlUtils.h"
//
//namespace test {
//
//namespace {
//
//void NextAnimFrame(Entity& girl)
//{
//	++girl.GetComponent<SpriteAnimationComponent>().index;
//}
//
//} // unnamed
//
///* Idle */
//void GirlIdleStateHandler::OnEnter(GirlStateContext& ctx)
//{
//	auto& anim = ctx.girl.GetComponent<SpriteAnimationComponent>();
//	anim.spriteSeriesName = "girl_idle";
//	anim.index.current = 0;
//}
//
//void GirlIdleStateHandler::UpdateAnimation(GirlStateContext& ctx)
//{
//	const auto& animDeltas = ctx.girl.GetComponent<AnimationDeltas>();
//
//	if (ctx.timeInCurrentAnimFrame >= animDeltas.idleTime)
//	{
//		NextAnimFrame(ctx.girl);
//	}
//}
//
///* Walk */
//void GirlWalkStateHandler::OnEnter(GirlStateContext& ctx)
//{
//	auto& anim = ctx.girl.GetComponent<SpriteAnimationComponent>();
//	anim.spriteSeriesName = "girl_walk";
//	anim.index.current = 0;
//}
//
//void GirlWalkStateHandler::UpdateAnimation(GirlStateContext& ctx)
//{
//	const auto& animDeltas = ctx.girl.GetComponent<AnimationDeltas>();
//
//	if (ctx.distanceInCurrentAnimFrame.x >= animDeltas.walkDeltaX)
//	{
//		NextAnimFrame(ctx.girl);
//	}
//}
//
///* Jump */
//void GirlJumpStateHandler::Upkeep(GirlStateContext& ctx)
//{
//	auto& girlState = ctx.girl.GetComponent<GirlState>();
//
//	if (girlState.jumpInitiated && CoyoteTimeFrameCounterActive())
//	{
//		++coyoteTimeFrameCounter_;
//	}
//
//	if (!CoyoteTimeFrameCounterActive())
//	{
//		girlState.jumpInitiated = false;
//	}
//}
//
//void GirlJumpStateHandler::OnEnter(GirlStateContext& ctx)
//{
//	auto [girlState, anim, rigid, targets] = 
//		ctx.girl.GetComponents<GirlState, SpriteAnimationComponent, RigidBody, MoveTargets>();
//
//	anim.spriteSeriesName = "girl_jump";
//	anim.index.current = 0;
//
//	const float jumpImpulseY = ComputeJumpImpulseY(rigid, targets.jumpVelY);
//
//	rigid.forceRequests.impulses.emplace_back(
//		Force{ .value = { 0.0f, -jumpImpulseY } });
//
//	EndCoyoteTimeFrameCounter();
//
//	girlState.jumpInitiated = false;
//}
//
//void GirlJumpStateHandler::UpdateAnimation(GirlStateContext& ctx)
//{
//	const auto& cGirl = ctx.girl;
//	auto [anim, animDeltas] = 
//		ctx.girl.GetComponents<SpriteAnimationComponent, AnimationDeltas>();
//
//	if (ctx.distanceInCurrentAnimFrame.y >= animDeltas.jumpDeltaY &&
//		anim.index.current < anim.index.max)
//	{
//		NextAnimFrame(ctx.girl);
//	}
//}
//
//bool GirlJumpStateHandler::CoyoteTimeFrameCounterActive() const noexcept
//{
//	return coyoteTimeFrameCounter_ <= kGirlCoyoteTimeFrameCount;
//}
//
///* Fall */
//void GirlFallStateHandler::OnEnter(GirlStateContext& ctx)
//{
//	auto [anim, rigid] = 
//		ctx.girl.GetComponents<SpriteAnimationComponent, RigidBody>();
//
//	anim.spriteSeriesName = "girl_fall";
//	anim.index.current = 0;
//
//	auto& body = GetWriteAccess(rigid.body);
//	body.SetGravityScale(2.5f);
//}
//
//void GirlFallStateHandler::OnExit(GirlStateContext& ctx)
//{
//	auto& rigid = ctx.girl.GetComponent<RigidBody>();
//	auto& body = GetWriteAccess(rigid.body);
//
//	body.SetGravityScale(1.0f);
//}
//
//void GirlFallStateHandler::UpdateAnimation(GirlStateContext& ctx)
//{
//	const auto& cGirl = ctx.girl;
//	auto [anim, animDeltas] =
//		ctx.girl.GetComponents<SpriteAnimationComponent, AnimationDeltas>();
//
//	if (ctx.distanceInCurrentAnimFrame.y >= animDeltas.fallDeltaY &&
//		anim.index.current < anim.index.max)
//	{
//		NextAnimFrame(ctx.girl);
//	}
//}
//
///* Land */
//void GirlLandStateHandler::OnEnter(GirlStateContext& ctx)
//{
//	auto [anim, rigid, collider] =
//		ctx.girl.GetComponents<SpriteAnimationComponent, RigidBody, Collider>();
//
//	anim.spriteSeriesName = "girl_land";
//	anim.index.current = 0;
//
//	auto& body = GetWriteAccess(rigid.body);
//	body.SetLinearVelocity({ 0.0f, 0.0f });
//
//	auto& shape = GetWriteAccess(collider.shape);
//	shape.SetFriction(kGirlColliderLandingFriction);
//} 
//
//void GirlLandStateHandler::OnExit(GirlStateContext& ctx)
//{
//	auto& rigid = ctx.girl.GetComponent<RigidBody>();
//	auto& body = GetWriteAccess(rigid.body);
//
//	body.SetGravityScale(1.0f);
//}
//
//void GirlLandStateHandler::UpdateAnimation(GirlStateContext& ctx)
//{
//	const auto& cGirl = ctx.girl;
//	auto [anim, animDeltas] =
//		ctx.girl.GetComponents<SpriteAnimationComponent, AnimationDeltas>();
//
//	if (ctx.timeInCurrentAnimFrame >= animDeltas.landTime &&
//		anim.index.current < anim.index.max)
//	{
//		NextAnimFrame(ctx.girl);
//	}
//}
//
///* Attack */
//void GirlAttackStateHandler::Upkeep(GirlStateContext& ctx)
//{
//	animDataRef_.Update(ctx.dt);
//}
//
//void GirlAttackStateHandler::OnEnter(GirlStateContext& ctx)
//{
//	animDataRef_.MarkNewAttack();
//
//	auto [anim, girlState, collider] =
//		ctx.girl.GetComponents<SpriteAnimationComponent, GirlState, Collider>();
//
//	anim.spriteSeriesName = animDataRef_.GetAttackAnimSeriesName();
//	anim.index.current = 0;
//
//	if (IsGirlOnGround(girlState))
//	{
//		auto& shape = GetWriteAccess(collider.shape);
//		shape.SetFriction(kGirlColliderLandingFriction);
//	}
//
//	girlState.attackInitiated = false;
//}
//
//void GirlAttackStateHandler::OnExit(GirlStateContext& ctx)
//{
//	auto& collider = ctx.girl.GetComponent<Collider>();
//
//	auto& shape = GetWriteAccess(collider.shape);
//	shape.SetFriction(kGirlColliderFriction);
//
//	animDataRef_.MarkAttackEnd();
//}
//
//void GirlAttackStateHandler::UpdateAnimation(GirlStateContext& ctx)
//{
//	const auto& cGirl = ctx.girl;
//	auto [anim, animDeltas] =
//		ctx.girl.GetComponents<SpriteAnimationComponent, AnimationDeltas>();
//
//	if (ctx.timeInCurrentAnimFrame >=
//		animDataRef_.GetAttackAnimChangeTime(animDeltas) &&
//		anim.index.current < anim.index.max)
//	{
//		NextAnimFrame(ctx.girl);
//	}
//}
//
//float GirlAttackStateHandler::GetAttackAnimChangeTime(const GirlStateContext& ctx) const
//{
//	if (!ctx.girl.IsValid())
//	{
//		return 0.0f;
//	}
//
//	const auto& animDeltas = ctx.girl.GetComponent<AnimationDeltas>();
//
//	return animDataRef_.GetAttackAnimChangeTime(animDeltas);
//}
//
//} // test