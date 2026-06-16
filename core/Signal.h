#pragma once
#include "../core/commonObjects.h"
#include "../deps/function2/function2.hpp"
#include "../core/Algorithms.h"

// MAKE SURE TO CLEAR OUT THE TOKENS BEFORE SIGNALS ARE DESTROYED! //

template <typename...Args>
class Signal;

class SignalToken
{
public:
	template <typename...Args> friend class Signal;
	template <typename PassKey, typename...Args> friend class PrivateSignal;

	friend struct std::hash<SignalToken>;

	SignalToken() = default;
	~SignalToken() { Disconnect(); }

	SignalToken(const SignalToken&) = delete;
	SignalToken& operator=(const SignalToken&) = delete;

	SignalToken(SignalToken&& other) noexcept : 
		id_(other.id_), disconnectView_(other.disconnectView_)
	{
		other.id_ = -1;
		other.disconnectView_ = nullptr;
	}
	SignalToken& operator=(SignalToken&& other) noexcept
	{
		if (this != &other)
		{
			id_ = other.id_;
			disconnectView_ = other.disconnectView_;
			other.id_ = -1;
			other.disconnectView_ = nullptr;
		}
		return *this;
	}

	bool IsConnected() const 
	{ 
		return id_ >= 0 && disconnectView_;
	}

	void Disconnect()
	{
		if (IsConnected())
		{
			disconnectView_(*this);
			id_ = -1;
			disconnectView_ = nullptr;
		}
	}

	bool operator==(const SignalToken& rhs) const {
		return id_ == rhs.id_;
	}

private:
	SignalToken(int id, fu2::function_view<void(SignalToken&)> disconnectView) :
		id_(id), disconnectView_(disconnectView) {}

	int id_ = -1;
	fu2::function_view<void(SignalToken&)> disconnectView_;
};

namespace std {
	template <>
	struct hash<SignalToken> {
		size_t operator()(const SignalToken& tk) const noexcept {
			return std::hash<int>{}(tk.id_);
		}
	};
}

class SignalIDGenerator
{
	template <typename...Args>
	friend class Signal;

	static int GenerateID()
	{
		static int currentId = 0;
		return currentId++;
	}
};

//// TODO: Think about not making each signal type have their own id generation?
template <typename...Args>
class Signal
{
public:
	using SlotCallbackType = fu2::unique_function<void(Args...)>;

	struct Slot
	{
		int id = -1;
		SlotCallbackType callback;
	};

	Signal() : slots_(), disconnectFn_(GetDisconnectLambda()) {}
	~Signal() = default;

	Signal(const Signal&) = delete;
	Signal& operator=(const Signal&) = delete;

	Signal(Signal&& other) noexcept : 
		slots_(std::move(other.slots_)), disconnectFn_(GetDisconnectLambda()) 
	{
		other.disconnectFn_ = nullptr;
	}
	Signal& operator=(Signal&& other) noexcept
	{
		if (this != &other)
		{
			slots_ = std::move(other.slots_);
			disconnectFn_ = GetDisconnectLambda();
			other.disconnectFn_ = nullptr;
		}
	}


	template <typename...Ts> requires (std::convertible_to<Ts, Args> && ...)
	void Emit(Ts&&...ts)
	{
		for (auto& slot : slots_)
		{
			if (slot.callback)
			{
				assert(slot.id >= 0);

				slot.callback(std::decay_t<Ts>(ts)...);
			}
		}
	}

	template <typename Fn> requires std::convertible_to<Fn, SlotCallbackType>
	SignalToken Connect(Fn&& fn)
	{
		int nextId = SignalIDGenerator::GenerateID();

		slots_.emplace_back(nextId, std::forward<Fn>(fn));

		return SignalToken{ nextId, fu2::function_view<void(SignalToken&)>{disconnectFn_} };
	}

	size_t GetRefCount() const { return slots_.size(); }

private:
	auto GetDisconnectLambda()
	{
		return [this](SignalToken& token) -> void {
			if (token.id_ == -1)
			{
				return;
			}

			core::EraseIf(slots_, [id = token.id_](const auto& slot) {
				return slot.id == id; 
			});
		};
	}

	std::vector<Slot> slots_;
	fu2::unique_function<void(SignalToken&)> disconnectFn_;
};

template <typename PassKey, typename...Args>
class PrivateSignal
{
public:
	using Slot = fu2::unique_function<void(Args...)>;

	PrivateSignal() : slot_(nullptr), disconnectFn_(GetDisconnectLambda()) {}
	~PrivateSignal() = default;

	PrivateSignal(const PrivateSignal&) = delete;
	PrivateSignal& operator=(const PrivateSignal&) = delete;

	PrivateSignal(PrivateSignal&& other) noexcept :
		slot_(std::move(other.slot_)), disconnectFn_(GetDisconnectLambda())
	{
		other.disconnectFn_ = nullptr;
	}
	PrivateSignal& operator=(PrivateSignal&& other) noexcept
	{
		if (this != &other)
		{
			slot_ = std::move(other.slot_);
			disconnectFn_ = GetDisconnectLambda();
			other.disconnectFn_ = nullptr;
		}
		return *this;
	}

	template <typename...Ts> requires (std::convertible_to<Ts, Args> && ...)
	void Emit(Ts&&...ts)
	{
		if (slot_)
		{
			std::invoke(slot_, std::decay_t<Ts>(ts)...);
		}
	}

	template <typename Fn> requires std::convertible_to<Fn, Slot>
	SignalToken Connect(PassKey, Fn&& fn)
	{
		if (slot_)
		{
			return {};
		}

		slot_ = std::forward<Fn>(fn);

		return SignalToken{ 0, fu2::function_view<void(SignalToken&)>{disconnectFn_} };
	}

	bool IsOccupied() const { return slot_ != nullptr; }

private:
	auto GetDisconnectLambda()
	{
		return [this](SignalToken& token) -> void {
			if (token.id_ == -1)
			{
				return;
			}

			assert(token.id_ == 0);

			slot_ = nullptr;
		};
	}

	Slot slot_;
	fu2::unique_function<void(SignalToken&)> disconnectFn_;
};