#pragma once
#include <bitset>
#include "../events/EventConcepts.h"
#include "../components/ComponentConcepts.h"

class EventDataBitset
{
public:
	using BitsetType = std::bitset<EventDataTypeList::size>;

	EventDataBitset() = default;
	explicit EventDataBitset(const BitsetType& bitset) : bitset_(bitset) {}
	EventDataBitset(bool onOrOff) : bitset_(onOrOff ? BitsetType{}.set() : BitsetType{}) {}

	void Reset() { bitset_.reset(); }
	void SetAll(bool tf) { (tf) ? bitset_.set() : bitset_.reset(); }

	template <SomeEventData...Ts> requires (sizeof...(Ts) > 0)
	void Set(bool tf) { (bitset_.set(static_cast<size_t>(Ts::eventType), tf) && ...); }

	void Set(uint32_t eventType, bool tf) { bitset_.set(static_cast<size_t>(eventType), tf); }

	template <SomeEventData...Ts> requires (sizeof...(Ts) > 0)
	bool Test() const { return (bitset_.test(static_cast<size_t>(Ts::eventType)) && ...); }

	bool Test(uint32_t eventType) const { return bitset_.test(static_cast<size_t>(eventType)); }

	template <SomeEventData...Ts> requires (sizeof...(Ts) > 0)
	bool TestAny() const { return (bitset_.test(static_cast<size_t>(Ts::eventType)) || ...); }

	const BitsetType& GetBitset() const { return bitset_; }

private:
	BitsetType bitset_;
};

class ComponentBitset
{
public:
	using BitsetType = ComponentSignature;

	ComponentBitset() = default;
	explicit ComponentBitset(BitsetType bitset) : bitset_(bitset) {}
	ComponentBitset(bool onOrOff) : bitset_(onOrOff ? 0xFFFFFFFFFFFFFFFF : 0) {}

	void Reset() { bitset_ = 0; }

	void SetAll(bool tf) { bitset_ = (tf) ? 0xFFFFFFFFFFFFFFFF : 0; }

	template <SomeComponent...Ts> requires (sizeof...(Ts) > 0)
	void Set(bool tf) 
	{ 
		if (tf)
		{
			bitset_ |= (Ts::componentBit | ...);
		}
		else
		{
			bitset_ &= ~(Ts::componentBit | ...);
		}
	}

	void Set(ComponentSignature sig, bool tf)
	{
		if (tf)
		{
			bitset_ |= sig;
		}
		else
		{
			bitset_ &= ~sig;
		}
	}

	template <SomeComponent...Ts> requires (sizeof...(Ts) > 0)
	constexpr bool Test() const 
	{ 
		constexpr ComponentSignature sig = (Ts::componentBit | ...);
		return (bitset_ & sig) == sig;
	}

	constexpr bool Test(ComponentSignature sig) const { return (bitset_ & sig) == sig; }

	template <SomeComponent...Ts> requires (sizeof...(Ts) > 0)
	constexpr bool TestAny() const { return (bitset_ & (Ts::componentBit | ...)) != 0; }

	constexpr bool TestAny(ComponentSignature sig) const { return (bitset_ & sig) != 0; }

	operator BitsetType() const { return bitset_; }
	BitsetType GetBitset() const { return bitset_; }

private:
	BitsetType bitset_ = 0;
};
