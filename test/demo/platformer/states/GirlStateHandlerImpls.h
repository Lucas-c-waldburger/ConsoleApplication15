//#pragma once
//#include "../GirlStateHandler.h"
//#include "AttackAnimDataReference.h"
//
//namespace test {
//
//class GirlIdleStateHandler : public GirlStateHandler<GirlState::Animation::Idle>
//{
//public:
//	GirlIdleStateHandler() = default;
//
//	void OnEnter(GirlStateContext& ctx) override;
//	void UpdateAnimation(GirlStateContext& ctx) override;
//};
//
//class GirlWalkStateHandler : public GirlStateHandler<GirlState::Animation::Walking>
//{
//public:
//	GirlWalkStateHandler() = default;
//
//	void OnEnter(GirlStateContext& ctx) override;
//	void UpdateAnimation(GirlStateContext& ctx) override;
//};
//
//class GirlJumpStateHandler : public GirlStateHandler<GirlState::Animation::Jumping>
//{
//public:
//	static constexpr size_t kGirlCoyoteTimeFrameCount = 4;
//
//	GirlJumpStateHandler() = default;
//
//	void Upkeep(GirlStateContext& ctx) override;
//	void OnEnter(GirlStateContext& ctx) override;
//	void UpdateAnimation(GirlStateContext& ctx) override;
//
//private:
//	void StartCoyoteTimeFrameCounter() { coyoteTimeFrameCounter_ = 0; }
//	void EndCoyoteTimeFrameCounter() { kGirlCoyoteTimeFrameCount + 1; }
//	bool CoyoteTimeFrameCounterActive() const noexcept;
//
//	size_t coyoteTimeFrameCounter_ = kGirlCoyoteTimeFrameCount + 1;
//};
//
//class GirlFallStateHandler : public GirlStateHandler<GirlState::Animation::Falling>
//{
//public:
//	GirlFallStateHandler() = default;
//
//	void OnEnter(GirlStateContext& ctx) override;
//	void OnExit(GirlStateContext& ctx) override;
//	void UpdateAnimation(GirlStateContext& ctx) override;
//};
//
//class GirlLandStateHandler : public GirlStateHandler<GirlState::Animation::Landing>
//{
//public:
//	GirlLandStateHandler() = default;
//
//	void OnEnter(GirlStateContext& ctx) override;
//	void OnExit(GirlStateContext& ctx) override;
//	void UpdateAnimation(GirlStateContext& ctx) override;
//};
//
//class GirlAttackStateHandler : public GirlStateHandler<GirlState::Animation::Attacking>
//{
//public:
//	GirlAttackStateHandler() = default;
//
//	void Upkeep(GirlStateContext& ctx) override;
//	void OnEnter(GirlStateContext& ctx) override;
//	void OnExit(GirlStateContext& ctx) override;
//	void UpdateAnimation(GirlStateContext& ctx) override;
//
//	float GetAttackAnimChangeTime(const GirlStateContext& ctx) const;
//
//private:
//	AttackAnimDataReference animDataRef_;
//};
//
//}