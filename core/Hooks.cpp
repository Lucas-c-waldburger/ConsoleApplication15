#include "Hooks.h"
#include "Logger.h"

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

// no default constructor for HookPoint, need to make array the hacky way
Hooks::Hooks() : hookList_([] {
	std::array<HookPoint, kMaxHooks> hookList{};
	HookPointBuilder builder;

	std::ranges::generate(hookList, builder);

	return hookList;
}()) {}

void Hooks::SetHookPointImpl(HookPoint::Identifier ident)
{
	assert(ident < hookList_.size());

	auto& hookPoint = hookList_[ident];
	hookPoint.RunAttached();
}

void Hooks::DetachImpl(HookPoint::Identifier ident, Handle<HookPoint::Attachment>& handle)
{
	assert(ident < hookList_.size());

	if (!handleFactory_.IsHandleValid(handle))
	{
		LOG_WARNING("Attachment Handle was Invalid");
		return;
	}

	auto& hookPoint = hookList_[ident];

	auto found = std::find_if(hookPoint.attachments_.begin(), hookPoint.attachments_.end(),
		[&handle](const auto& attachment) { return attachment.handle == handle; });

	if (found != hookPoint.attachments_.end())
	{
		hookPoint.attachments_.erase(found);
		Hooks::Get().handleFactory_.RetireHandle(handle);
	}
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
