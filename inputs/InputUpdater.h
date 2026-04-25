#pragma once
#include "InputMap.h"
#include <bitset>

template <typename Derived, typename InpMap>
class InputUpdater;

template <typename Derived, typename Src, typename Val>
class InputUpdater<Derived, InputMap<Src, Val>>
{
public:
	using MapType = InputMap<Src, Val>;

	InputUpdater() : inputs_(MakeInputMap<MapType>()) {}
	~InputUpdater() = default;

	template <typename...Args>
	void Update(Args&&...args);

	template <typename...Args>
	void FinalizeAndPushEvents(Args&&...args);

	MapType& GetInputMap() { return inputs_; }
	const MapType& GetInputMap() const { return inputs_; }

protected:
	class Tracker
	{
	public:
		constexpr Tracker() : updated() { timestamps.fill(0); }

		std::bitset<enum_size_v<Src>> updated;
		std::array<uint32_t, enum_size_v<Src>> timestamps;
	};

	MapType inputs_;
	Tracker tracker_;
};

template <typename Derived, typename Src, typename Val>
template <typename...Args>
void InputUpdater<Derived, InputMap<Src, Val>>::Update(Args&&...args)
{
	static_cast<Derived*>(this)->UpdateImpl(std::forward<Args>(args)...);
}

template <typename Derived, typename Src, typename Val>
template <typename...Args>
void InputUpdater<Derived, InputMap<Src, Val>>::FinalizeAndPushEvents(Args&&...args)
{
	static_cast<Derived*>(this)->FinalizeAndPushEventsImpl(std::forward<Args>(args)...);
}