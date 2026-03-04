//#include "GirlStateHandler.h"
//#include "states/GirlStateHandlerImpls.h"
//
//class Entity;
//class EventBus2;
//
//namespace test {
//
//class GirlStateHandlers
//{
//public:
//	using State = GirlState::Animation;
//
//	explicit GirlStateHandlers(EventBus2& bus);
//
//	template <SomeGirlStateHandler T, typename...Args> 
//		requires std::constructible_from<T, Args...>
//	void RegisterState(Args&&...args);
//
//	void SetState(State newState);
//
//	State GetActiveState() const noexcept { return currentState_; }
//
//	std::unique_ptr<BaseGirlStateHandler>& GetActiveStateHandler();
//
//	template <SomeGirlStateHandler T>
//	T& GetStateHandler();
//
//	template <SomeGirlStateHandler T>
//	const T& GetStateHandler() const;
//
//	void Upkeep(Entity& girl, float dt);
//
//	void UpdateAnimation();
//
//	void Cleanup();
//
//	GirlStateContext& GetContext() { return ctx_; }
//	const GirlStateContext& GetContext() const { return ctx_; }
//
//private:
//	using Handlers = std::array<std::unique_ptr<BaseGirlStateHandler>, enum_size_v<State>>;
//
//	void SetFallbackState();
//
//	State currentState_ = State::Idle;
//	Handlers handlers_;
//	GirlStateContext ctx_;
//	EventBus2* eventBus_ = nullptr;
//};
//
//template <SomeGirlStateHandler T, typename...Args> requires std::constructible_from<T, Args...>
//void GirlStateHandlers::RegisterState(Args&&...args)
//{
//	static constexpr size_t stateIdx = static_cast<size_t>(T::GetStateIdentity());
//
//	handlers_[stateIdx] = std::make_unique<T>(std::forward<Args>(args)...);
//}
//
//template <SomeGirlStateHandler T>
//T& GirlStateHandlers::GetStateHandler()
//{
//	static constexpr size_t stateIdx = static_cast<size_t>(T::stateIdentity);
//
//	assert(handlers_[stateIdx]);
//
//	return static_cast<T&>(*handlers_[stateIdx]);
//}
//
//template <SomeGirlStateHandler T>
//const T& GirlStateHandlers::GetStateHandler() const
//{
//	static constexpr size_t stateIdx = static_cast<size_t>(T::stateIdentity);
//
//	assert(handlers_[stateIdx]);
//
//	return static_cast<const T&>(*handlers_[stateIdx]);
//}
//
//} // test