#include "ScriptSystem.h"
#include "../ecs/Ecs.h"

namespace {

bool ValidScript(const Script& script)
{
	return script.stateHandle.IsValid();
}

} // unnamed

ScriptSystem::ScriptSystem()
{
	ObserveEntityDestroyed();
	ObserveComponentRemoved();
}

void ScriptSystem::OnEntityDestroyed(Entity e)
{
	if (!e.HasComponent<Script>(&ValidScript))
	{
		return;
	}

	auto& script = e.GetComponent<Script>();

	stateMap_.erase(script.stateHandle);

	script.stateHandle = {};
	script.table = {};
}

void ScriptSystem::OnComponentRemoved(Entity e, ComponentSignature sig)
{
	if (sig != Script::componentBit)
	{
		return;
	}

	OnEntityDestroyed(e);

	if (e.HasComponent<SignalTokenStorage>())
	{
		auto& tks = e.GetComponent<SignalTokenStorage>().signalTokens;

		core::EraseIf(tks, [](const auto& tk) {
			return tk.type == EntityCallbackToken::Type::Script; }
		);
	}
}
