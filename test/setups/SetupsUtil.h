#pragma once
#include "../Fixtures.h"
#include "../../callbacks/EventCallback.h"

namespace test {

//template <typename Fn> requires EventCallbackFnCompatible<Fn>
//auto TimerCallbackSubmissionMaker(std::string_view name, Fn&& fn, float sec,
//								  Timer::Flag flags = Timer::Flag::RemoveOnExpiry)
//{
//
//}


template <typename Fn, typename T>
concept EventCallbackFnOfType = EventCallbackFnCompatible<Fn> && SomeEventData<T> &&
	ExtractEventDataTypeFromFnArgs<Fn>::eventType == T::eventType;

template <typename Fn> requires std::invocable<Fn>
Result<Void> SpinTimer(Entity& entity, EventCallbackRegistry& evRegistry, float sec, 
					   Fn&& internalFn, Timer::Flag flags = Timer::Flag::RemoveOnExpiry)
{
	if (!entity.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}
	if (!internalFn)
	{
		return MAKE_ERROR("Function argument was null");
	}

	auto relations = entity.GetRelations();
	if (relations.IsChild())
	{
		return MAKE_ERROR("Only parent entities can spin timers");
	}

	auto timerChild = relations.AddChild();
	timerChild.AddComponent(Timer{
		.duration = sec,
		.flags = static_cast<Timer::Flag>(Timer::Flag::Active | flags)
	});
 	
	auto& parentEvCallbacks = entity.AddComponent<EventCallbacks>().table;

	std::string timerName = std::format("timer_callback_{}_{}", 
		entity.GetID(), timerChild.GetID());

	if (evRegistry.HasCallback(events::TimerFired::eventType, timerName))
	{
		return MAKE_ERROR_FMT("Timer callback name already exists in registry: '{}'", 
			timerName);
	}

	auto evHandle = evRegistry.RegisterCallback(timerName,
		[childId = timerChild.GetID(), fn = std::forward<Fn>(internalFn)]
		(Entity& entity, const events::TimerFired& ev) -> ReturnSignal
		{
			if (ev.producer != childId)
			{
				return ReturnSignal::KeepObserving;
			}

			//auto parent = childEntity.GetRelations().GetParent();
			//if (!parent.IsValid())
			//{
			//	LOG_ERROR("Timer child did not have a valid parent");
			//	return ReturnSignal::StopObserving;
			//}

			//if (!fn)
			//{
			//	LOG_ERROR("Timer function was null");
			//	return ReturnSignal::StopObserving;
			//}

			std::invoke(fn);

			return ReturnSignal::StopObserving;
		});

	if (!evHandle.IsValid())
	{
		return MAKE_ERROR_FMT("Registration failed for timer callback '{}'", timerName);
	}

	parentEvCallbacks[evHandle.GetEventType()].push_back(evHandle);

	return Void{};
}

//template <typename...Drivers>
//class Workspace : public Drivers...
//{};





}