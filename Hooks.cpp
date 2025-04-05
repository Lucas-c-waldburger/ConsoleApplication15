#include "Hooks.h"

void Hook::RunAttached()
{
	if ((flags & HookManager::Registered) == 0)
	{
		LOG_WARNING("Hook was not registered");
		return;
	}
	if ((flags & HookManager::Active) == 0)
	{
		return;
	}

	for (auto& [_, func] : attached_)
	{
		if (!func)
		{
			LOG_WARNING("Attached function was null");
			continue;
		}

		func();
	}
}

void HookManager::DetachImpl(HookPoint hp, std::string_view name)
{
	auto& hook = hookList_[static_cast<size_t>(hp)];

	assert(hook);
	assert(hook->flags & HookFlag::Registered);

	if (hook->attached_.erase(std::string{name}) == 0)
	{
		LOG_WARNING_FMT("No attachment named '{}'", name);
	}
}
