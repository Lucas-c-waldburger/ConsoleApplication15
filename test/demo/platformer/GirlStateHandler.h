//#pragma once
//#include "Common.h"
//#include "../../../core/ReadOnly.h"
//
//class Entity;
//class EventBus2;
//
//namespace test {
//
//class BaseGirlStateHandler
//{
//public:
//	using State = GirlState::Animation;
//
//	BaseGirlStateHandler() = default;
//	virtual ~BaseGirlStateHandler() = default;
//
//	virtual void Upkeep(GirlStateContext&) {};
//	virtual void OnEnter(GirlStateContext&) {};
//	virtual void OnExit(GirlStateContext&) {};
//	virtual void UpdateAnimation(GirlStateContext&) {};
//
//	virtual SDL_FPoint GetMoveImpulseModifier() const { return { 1.0f, 1.0f }; }
//
//	State GetState() const { return state_; }
//
//protected:
//	explicit BaseGirlStateHandler(State st) : state_(st) {}
//
//private:
//	State state_;
//};
//
////BaseGirlStateHandler::~BaseGirlStateHandler() {}
//
//
//template <GirlState::Animation st>
//struct GirlStateIdentity
//{
//	static constexpr GirlState::Animation stateIdentity = st;
//	static constexpr GirlState::Animation GetStateIdentity()
//	{
//		return st;
//	}
//};
//
//template <GirlState::Animation st>
//class GirlStateHandler : public BaseGirlStateHandler, public GirlStateIdentity<st>,
//						 public HasWriteAccessImpl<BaseGirlStateHandler, B2Body, B2Shape>
//{
//public:
//	GirlStateHandler() : BaseGirlStateHandler(st) {}
//};
//
//template <typename T>
//concept SomeGirlStateHandler = requires {
//	std::derived_from<T, BaseGirlStateHandler>;
//	{ T::stateIdentity } -> std::convertible_to<GirlState::Animation>;
//	//{ T::GetStateIdentity() } -> std::convertible_to<GirlState::Animation>;
//};
//
//
//} // test