#pragma once
#include "EventStage.h"
#include "../core/Algorithms.h"
#include "../deps/function2/function2.hpp"

using EventDispatchListenerFn = fu2::unique_function<void(EventSpan)>;

class EventDispatchListenerToken
{
public:
	friend class EventDispatchListeners;

	static constexpr int kInvalidId = -1;

	EventDispatchListenerToken() = default;
	~EventDispatchListenerToken() { Disconnect(); }

	EventDispatchListenerToken(const EventDispatchListenerToken&) = delete;
	EventDispatchListenerToken& operator=(const EventDispatchListenerToken&) = delete;

	EventDispatchListenerToken(EventDispatchListenerToken&& rhs) noexcept : id_(rhs.id_)
	{
		rhs.id_ = kInvalidId;
	}

	EventDispatchListenerToken& operator=(EventDispatchListenerToken&& other) noexcept
	{
		if (this != &other)
		{
			Disconnect();
			id_ = other.id_;
			other.id_ = kInvalidId;
		}
		return *this;
	}


	bool IsConnected() const { return id_ != kInvalidId; }
	void Disconnect();

private:
	explicit EventDispatchListenerToken(int id) : id_(id) {}

	int id_ = kInvalidId;
};


class EventDispatchListeners
{
public:
	template <typename Fn>
	EventDispatchListenerToken AddListener(Fn&& fn)
	{
		int newId = idCounter_++;

		listeners_.emplace_back(newId, std::forward<Fn>(fn));

		return EventDispatchListenerToken{ newId };
	}

	void RemoveListener(const EventDispatchListenerToken& token)
	{
		if (token.IsConnected())
		{
			core::EraseIf(listeners_, [id = token.id_](const auto& pair) {
				return pair.first == id;
			});
		}
	}

	bool Size() const { return listeners_.size(); }
	bool Empty() const { return listeners_.empty(); }

	auto begin() { return listeners_.begin(); }
	auto end() { return listeners_.end(); }

private:
	std::vector<std::pair<int, EventDispatchListenerFn>> listeners_;
	int idCounter_ = 0;
};