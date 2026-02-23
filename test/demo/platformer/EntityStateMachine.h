#pragma once
#include "EntityStateNode.h"
#include "../../../core/SizedEnum.h"
#include <array>

namespace test {

template <typename NodeType, size_t =
	(SomeSizedEnum<typename NodeType::StateEnumType> ?
	 enum_size_v<typename NodeType::StateEnumType> : 0)>
struct EntityStateMachine;

template <typename StateEnum, typename Ctx, size_t StateCount>
struct EntityStateMachine<EntityStateNode<StateEnum, Ctx>, StateCount>
{
	static_assert(StateCount > 0);

	static constexpr size_t stateCount = StateCount;
	using NodeType = EntityStateNode<StateEnum, Ctx>;
	using StateEnumType = StateEnum;
	using ContextType = Ctx;
	using EvaluationResponse = StateEvaluationResponse<StateEnum>;

	EntityStateMachine() = default;
	explicit EntityStateMachine(StateEnumType startingState) : current(startingState) {}

	void Insert(StateEnumType st, NodeType node)
	{
		assert(node.evaluateState);

		const auto stateIdx = static_cast<size_t>(st);
		assert(stateIdx < stateNodes.size());

		stateNodes[stateIdx] = std::move(node);
	}

	NodeType& operator[](StateEnumType st)
	{
		const auto stateIdx = static_cast<size_t>(st);
		assert(stateIdx < stateNodes.size());

		return stateNodes[stateIdx];
	}

	const NodeType& operator[](StateEnumType st) const
	{
		const auto stateIdx = static_cast<size_t>(st);
		assert(stateIdx < stateNodes.size());

		return stateNodes[stateIdx];
	}

	template <typename...Ts>
	void Evaluate(Ts&&...args)
	{
		StateEvaluationResponse<StateEnumType> response{};

		do {
			const auto stateIdx = static_cast<size_t>(current);
			assert(stateIdx < stateNodes.size());

			auto& state = stateNodes[stateIdx];
			if (!state.evaluateState)
			{
				return;
			}

			if constexpr (std::same_as<ContextType, Void>)
			{
				static_assert(std::invocable<typename NodeType::EvalFnSig,
					Ts...>, "Evaluate could not be invoked by the provided arguments");

				response = (*state.evaluateState)(std::forward<Ts>(args)...);
			}
			else
			{
				static_assert(std::invocable<typename NodeType::EvalFnSig,
					ContextType&, Ts...>,
					"Evaluate could not be invoked by the provided arguments"
					"Did you include 'ContextType' as the first argument?");

				response = (*state.evaluateState)(context, std::forward<Ts>(args)...);
			}

			if (response.state == current)
			{
				assert(!response.reEvaluate);
				return;
			}

			if (state.onExit)
			{
				if constexpr (std::same_as<ContextType, Void>)
				{
					static_assert(std::invocable<typename NodeType::CommonFnSig,
						Ts...>, "OnExit could not be invoked by the provided arguments");

					(*state.onExit)(std::forward<Ts>(args)...);
				}
				else
				{
					static_assert(std::invocable<typename NodeType::CommonFnSig,
						ContextType&, Ts...>,
						"OnExit could not be invoked by the provided arguments"
						"Did you include 'ContextType' as the first argument?");

					(*state.onExit)(context, std::forward<Ts>(args)...);
				}
			}

			current = response.state;

			const auto newStateIdx = static_cast<size_t>(current);
			assert(newStateIdx < stateNodes.size());

			auto& newState = stateNodes[newStateIdx];
			if (newState.onEnter)
			{
				if constexpr (std::same_as<ContextType, Void>)
				{
					static_assert(std::invocable<typename NodeType::CommonFnSig,
						Ts...>, "OnEnter could not be invoked by the provided arguments");

					(*newState.onEnter)(std::forward<Ts>(args)...);
				}
				else
				{
					static_assert(std::invocable<typename NodeType::CommonFnSig,
						ContextType&, Ts...>,
						"OnEnter could not be invoked by the provided arguments"
						"Did you include 'ContextType' as the first argument?");

					(*newState.onEnter)(context, std::forward<Ts>(args)...);
				}
			}
		} while (response.reEvaluate);
	}

	template <typename...Ts>
	void UpdateIntents(Ts&&...args)
	{
		const auto stateIdx = static_cast<size_t>(current);
		assert(stateIdx < stateNodes.size());

		auto& state = stateNodes[stateIdx];
		if (!state.updateIntents)
		{
			return;
		}

		if constexpr (std::same_as<ContextType, Void>)
		{
			static_assert(std::invocable<typename NodeType::CommonFnSig,
				Ts...>, "UpdateIntents could not be invoked by the provided arguments");

			(*state.updateIntents)(std::forward<Ts>(args)...);
		}
		else
		{
			static_assert(std::invocable<typename NodeType::CommonFnSig,
				ContextType&, Ts...>,
				"UpdateIntents could not be invoked by the provided arguments"
				"Did you include 'ContextType' as the first argument?");

			(*state.updateIntents)(context, std::forward<Ts>(args)...);
		}
	}

	template <typename...Ts>
	void UpdateAnimations(Ts&&...args)
	{
		const auto stateIdx = static_cast<size_t>(current);
		assert(stateIdx < stateNodes.size());

		auto& state = stateNodes[stateIdx];
		if (!state.updateAnimations)
		{
			return;
		}

		if constexpr (std::same_as<ContextType, Void>)
		{
			static_assert(std::invocable<typename NodeType::CommonFnSig,
				Ts...>, "UpdateAnimations could not be invoked by the provided arguments");

			(*state.updateAnimations)(std::forward<Ts>(args)...);
		}
		else
		{
			static_assert(std::invocable<typename NodeType::CommonFnSig,
				ContextType&, Ts...>,
				"UpdateAnimations could not be invoked by the provided arguments"
				"Did you include 'ContextType' as the first argument?");

			(*state.updateAnimations)(context, std::forward<Ts>(args)...);
		}
	}

	std::array<NodeType, StateCount> stateNodes;
	StateEnumType current = static_cast<StateEnumType>(0);
	ContextType context;
};

} // test