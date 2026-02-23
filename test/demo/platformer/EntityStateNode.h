#pragma once
#include "../../../core/commonObjects.h"

class Entity;

namespace test {

template <typename StateEnum>
struct StateEvaluationResponse
{
	StateEvaluationResponse() = default;
	StateEvaluationResponse(StateEnum e) : state(e) {}
	StateEvaluationResponse(StateEnum e, bool tf) : state(e), reEvaluate(tf) {}

	StateEnum state;
	bool reEvaluate = false;
};

template <typename StateEnum, typename Ctx = Void>
	requires (std::is_enum_v<StateEnum>&&
std::is_unsigned_v<std::underlying_type_t<StateEnum>>)
struct EntityStateNode
{
	using StateEnumType = StateEnum;
	using ContextType = Ctx;
	using EvaluationResponse = StateEvaluationResponse<StateEnumType>;

	using CommonFnSig = std::conditional_t<std::same_as<Ctx, Void>,
		void(*)(Entity&), void(*)(Ctx&, Entity&)
	>;
	using EvalFnSig = std::conditional_t<std::same_as<Ctx, Void>,
		EvaluationResponse(*)(Entity&), EvaluationResponse(*)(Ctx&, Entity&)
	>;

	CommonFnSig onEnter = nullptr;
	EvalFnSig   evaluateState = nullptr;
	CommonFnSig updateIntents = nullptr;
	CommonFnSig updateAnimations = nullptr;
	CommonFnSig onExit = nullptr;
};

} // test