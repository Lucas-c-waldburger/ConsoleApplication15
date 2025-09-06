#pragma once
#include <SDL_timer.h>
#include "EventDataStorage.h"
#include "EventDispatchListener.h"


class EventBus
{
public:
	using EventStage = EventStage<256>;

	~EventBus() = default;
	EventBus(const EventBus&) = delete;
	EventBus(EventBus&&) = delete;
	EventBus& operator=(const EventBus&) = delete;
	EventBus& operator=(EventBus&&) = delete;

	template <SomeEventData T>
	static void PushEvent(T&& evData)
	{
		evData.timestamp = SDL_GetTicks();
		Get()->storage_.Emplace(std::forward<T>(evData));
	}

	template <SomeEventData T>
	static const std::vector<T>& PeekEvents()
	{
		return Get()->storage_.GetStorageEntry<T>();
	}

	template <SomeEventData...Ts>
	static void DispatchEvents()
	{
		auto stageAll = []<typename T>(auto & storage, auto & stage) {
			const auto& eventsEntry = storage.GetStorageEntry<T>();
			if (eventsEntry.empty())
			{
				return;
			}

			stage.StageEvents(eventsEntry);
		};

		auto& self = Get();

		((stageAll.template operator()<Ts>(self->storage_, self->stage_)), ...);

		auto stagedEvents = self->stage_.GetStagedEvents();
		if (stagedEvents.empty())
		{
			return;
		}

		for (auto& [_, listener] : self->dispatchListeners_)
		{
			if (listener)
			{
				listener(stagedEvents);
			}
		}

		self->stage_.ClearStage();
		((self->storage_.Clear<Ts>()), ...);
	}

	// unpack an EventGroup for DispatchEvents
	struct UnpackGroupAndDispatch
	{
		template <typename...Ts>
		static void Apply()
		{
			EventBus::template DispatchEvents<Ts...>();
		}
	};

	template <typename Group>
	static void DispatchEventGroup()
	{
		Group::template Apply<UnpackGroupAndDispatch>();
	}

	static void ClearEvents()
	{
		auto& self = Get();

		self->stage_.ClearStage();
		self->storage_.ClearAll();
	}

	template <typename Fn>
	[[nodiscard]] static EventDispatchListenerToken ConnectDispatchListener(Fn&& fn)
	{
		return Get()->dispatchListeners_.AddListener(std::forward<Fn>(fn));
	}

	static void DisconnectDispatchListener(const EventDispatchListenerToken& token)
	{
		Get()->dispatchListeners_.RemoveListener(token);
	}

private:
	EventBus() = default;

	static std::unique_ptr<EventBus>& Get()
	{
		static std::unique_ptr<EventBus> instance;
		if (!instance)
		{
			instance = std::unique_ptr<EventBus>(new EventBus{});
		}

		return instance;
	}

	EventDataStorage storage_{};
	EventStage stage_{};
	EventDispatchListeners dispatchListeners_;
};