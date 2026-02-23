#include "Common.h"
#include "../../../ecs/Ecs.h"

namespace test {

EvaluatedGirlStateContext 
GirlStateContext::Evaluate(const Entity& girl, const GirlStateContext& ctx)
{
	if (!girl.HasComponents<Transform, GirlState, GirlIntent>())
	{
		return {};
	}

	const auto [tf, state, intents, gc] =
		girl.GetComponents<Transform, GirlState, 
						   GirlIntent, GameControllerState>();

	const auto& gcState =
		gc.inputs[GameControllerInputSource::LeftStickAxis].state;

	//if (!(EqualsWithTolerance(intents.moveIntent.x, 0.0f) &&
	//	EqualsWithTolerance(intents.moveIntent.y, 0.0f)))
	//{
	//	int i = 0;
	//}
	if (!(EqualsWithTolerance(tf.position.x, ctx.deltas.lastPosition.x) &&
		EqualsWithTolerance(tf.position.y, ctx.deltas.lastPosition.y)))
	{
		int i = 0;
	}

	return {
		.onGround = state.collidingCategories[ObjectCategory::Ground] > 0,

		.moved = !(EqualsWithTolerance(tf.position.x, ctx.deltas.lastPosition.x) &&
				   EqualsWithTolerance(tf.position.y, ctx.deltas.lastPosition.y)),

		.jumpPressed = intents.jumpIntent == InputState::Pressed,

		.attackPressed = intents.attackIntent == InputState::Pressed || 
						 intents.attackIntent == InputState::Held,

		.thumbstickEngaged = gcState == InputState::Pressed || gcState == InputState::Held
	};
}

} // test