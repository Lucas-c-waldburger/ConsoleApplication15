//#pragma once
//#include "GirlStateHandlers.h"
//#include "states/GirlStateHandlerImpls.h"
//
//class EventBus2;
//
//namespace test {
//
//class GirlStateCoordinator
//{
//public:
//	using State = GirlState::Animation;
//
//	GirlStateCoordinator(Entity_t girlId, EventBus2& bus);
//
//	void Update(float dt);
//
//private:
//	void HandleAxisMoveIntent();
//	void UpdateSpriteFacingSide();
//
//	Entity_t girlId_ = kInvalidEntity;
//	GirlStateHandlers stateHandlers_;
//};
//
//} // test
////#include "GirlStateFunctions.h"
////#include "../../../ecs/Ecs.h"
////
////class SystemManager;
////
////namespace test {
////
////using GirlStateMachine = EntityStateMachine<GirlStateNode>;
////
////class GirlStateCoordinator
////{
////public:
////	using enum GirlState::Animation;
////
////	class EvaluationSystem;
////	class IntentSystem;
////	class AnimationSystem;
////
////	explicit GirlStateCoordinator(Entity_t girlId);
////
////	void RegisterSystems(SystemManager& sysManager);
////
////private:
////	template <typename Derived> class BaseSystem;
////
////	void UpdateStateContext(Entity& e, float dt);
////	void ResolveStateContext(Entity& e);
////
////	Entity_t girlId_ = kInvalidEntity;
////	GirlStateMachine stateMachine_;
////};
////
////
////template <typename Derived>
////class GirlStateCoordinator::BaseSystem
////{
////public:
////	explicit BaseSystem(GirlStateCoordinator& parent) :
////		parentCoordinator_(&parent) {}
////
////	void Update(float dt)
////	{
////		if (!parentCoordinator_) { return; }
////
////		auto girl = ECS::GetEntityByID(parentCoordinator_->girlId_);
////
////		if (!girl.IsValid()) { return; }
////
////		static_cast<Derived*>(this)->UpdateImpl(girl, dt);
////	}
////
////protected:
////	GirlStateCoordinator* parentCoordinator_ = nullptr;
////};
////
////class GirlStateCoordinator::EvaluationSystem : public BaseSystem<EvaluationSystem>
////{
////public:
////	explicit EvaluationSystem(GirlStateCoordinator& parent) : BaseSystem(parent) {}
////	void UpdateImpl(Entity& girl, float dt);
////};
////
////class GirlStateCoordinator::IntentSystem : public BaseSystem<IntentSystem>
////{
////public:
////	explicit IntentSystem(GirlStateCoordinator& parent) : BaseSystem(parent) {}
////	void UpdateImpl(Entity& girl, float);
////};
////
////class GirlStateCoordinator::AnimationSystem : public BaseSystem<AnimationSystem>
////{
////public:
////	explicit AnimationSystem(GirlStateCoordinator& parent) : BaseSystem(parent) {}
////	void UpdateImpl(Entity& girl, float);
////
////private:
////	static void UpdateSpriteFacingSide(Entity& girl, SDL_FPoint lastPos);
////};
////
////
////} // test