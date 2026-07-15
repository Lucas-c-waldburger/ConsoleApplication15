#pragma once
#include <bitset>
#include "../events/EventConcepts.h"
#include "../components/ComponentConcepts.h"

template <typename>
class TypeIndexedBitset;

template <template <typename...> class TList, typename...Ts>
class TypeIndexedBitset<TList<Ts...>>
{
private:
	using Types = TList<Ts...>;
	using BitsetType = std::bitset<sizeof...(Ts)>;

	template <typename T>
	static constexpr bool type_in_bitset_v = (std::same_as<T, Ts> || ...);

	template <typename T> requires type_in_bitset_v<T>
	static constexpr size_t index_v = index_of_v<T, Types>;

	BitsetType bitset_;

public:
	constexpr TypeIndexedBitset() = default;
	explicit constexpr TypeIndexedBitset(bool tf) : bitset_(tf ? BitsetType{}.set() : BitsetType{}) {}

	template <typename T> requires type_in_bitset_v<T>
	void Set(bool tf) { bitset_.set(index_v<T>, tf); }

	template <typename T> requires type_in_bitset_v<T>
	constexpr bool Test() const { return bitset_.test(index_v<T>); }

	void Reset() { bitset_.reset(); }
};


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
