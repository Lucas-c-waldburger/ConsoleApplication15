#pragma once
#include "../Fixtures.h"
#include "../callbacks/AnimationCallbacks.h"
#include "../Resources.h"

namespace test {


Result<Void> SetUpWalkResetProxyChild(Entity& knight, EventCallbackRegistry& evRegistry)
{
	assert(knight.IsValid());

	TRY(knight.TryGetComponent<Transform>(), knightTf);
	TRY(knight.TryGetComponent<EventCallbacks>(), knightEvCallbacks);

	auto walkResetCallbackHandle = evRegistry.RegisterOrRetrieveCallback(
		NAME_AND_CALL(ResetWalkSpriteIfNoMovementAndUpdate)
	);
	assert(walkResetCallbackHandle.IsValid());

	knightEvCallbacks.get().table[walkResetCallbackHandle.GetEventType()].emplace_back(
		walkResetCallbackHandle
	);

	auto proxy = knight.GetRelations().AddProxyChild(kLastTransformProxyChildName);
	assert(proxy.IsValid());

	auto& proxyTf = proxy.AddComponent<Transform>();
	proxyTf = knightTf;

	return Void{};
}







}