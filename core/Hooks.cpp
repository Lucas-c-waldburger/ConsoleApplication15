#include "Hooks.h"

void HookPoint::Detach(Handle<HookPoint::Attachment>& handle)
{
	if (!Hooks::Get().handleFactory_.IsHandleValid(handle))
	{
		return;
	}

	auto found = std::find_if(attachments_.begin(), attachments_.end(),
		[&handle](const auto& attachment) { return attachment.handle == handle; });

	if (found != attachments_.end())
	{
		attachments_.erase(found);
		Hooks::Get().handleFactory_.RetireHandle(handle);
	}
}

void HookPoint::RunAttached()
{
	auto it = attachments_.begin();
	while (it != attachments_.end())
	{
		if (!(Hooks::Get().handleFactory_.IsHandleValid(it->handle) && it->callback))
		{
			it = attachments_.erase(it);
		}
		else
		{
			it->callback();
			++it;
		}
	}
}

void Hooks::SetImpl(HookPoint::Identifier ident)
{
	assert(ident < hookList_.size());

	auto& hookPoint = hookList_[ident];
	if (!hookPoint)
	{
		hookPoint = std::make_unique<HookPoint>();
	}

	hookPoint->RunAttached();
}

std::unique_ptr<HookPoint>& Hooks::GetHookPointImpl(HookPoint::Identifier ident)
{
	assert(ident < hookList_.size());

	auto& hookPoint = hookList_[ident];
	if (!hookPoint)
	{
		hookPoint = std::make_unique<HookPoint>();
	}

	return hookPoint;
}

Hooks& Hooks::Get()
{
	static std::unique_ptr<Hooks> instance;
	if (!instance)
	{
		instance = std::unique_ptr<Hooks>(new Hooks{});
	}

	return *instance;
}


//void Hook::RunAttached()
//{
//	if ((flags & HookManager::Registered) == 0)
//	{
//		LOG_WARNING("Hook was not registered");
//		return;
//	}
//	if ((flags & HookManager::Active) == 0)
//	{
//		return;
//	}
//
//	for (auto& [_, func] : attached_)
//	{
//		if (!func)
//		{
//			LOG_WARNING("Attached function was null");
//			continue;
//		}
//
//		func();
//	}
//}
//
//void HookManager::DetachImpl(HookPoint hp, std::string_view name)
//{
//	auto& hook = hookList_[static_cast<size_t>(hp)];
//
//	assert(hook);
//	assert(hook->flags & HookFlag::Registered);
//
//	if (hook->attached_.erase(std::string{name}) == 0)
//	{
//		LOG_WARNING_FMT("No attachment named '{}'", name);
//	}
//}


