#pragma once
#include <bitset>
#include <array>
#include "../events/EventConcepts.h"
#include "../components/ComponentConcepts.h"

template <typename>
class TypeIndexedBitset;

template <template <typename...> class TList, typename...Ts>
class TypeIndexedBitset<TList<Ts...>>
{
private:
	using BitsetType = std::bitset<sizeof...(Ts)>;

	BitsetType bitset_;

	template <typename T>
	static constexpr bool type_in_bitset_v = (std::same_as<T, Ts> || ...);

	template <typename T> requires type_in_bitset_v<T>
	static constexpr size_t index_v = index_of_v<T, TList<Ts...>>;

public:
	constexpr TypeIndexedBitset() = default;
	explicit constexpr TypeIndexedBitset(bool tf) : bitset_(tf ? BitsetType{}.set() : BitsetType{}) {}

	template <typename T> requires type_in_bitset_v<T>
	void Set(bool tf) { bitset_.set(index_v<T>, tf); }

	template <typename T> requires type_in_bitset_v<T>
	constexpr bool Test() const { return bitset_.test(index_v<T>); }

	void Reset() { bitset_.reset(); }

	template <typename T> requires type_in_bitset_v<T>
	constexpr size_t IndexOf() const { return index_v<T>; }
};

template <typename, typename>
class TypeIndexedBitMap;

template <template <typename...> class TList, typename ValT, typename...Ts>
class TypeIndexedBitMap<TList<Ts...>, ValT>
{
private:
	using BitsetType = std::bitset<sizeof...(Ts)>;
	using ArrayType = std::array<ValT, sizeof...(Ts)>;

	BitsetType bitset_;
	ArrayType values_;

	template <typename T>
	static constexpr bool type_in_bitset_v = (std::same_as<T, Ts> || ...);

	template <typename T> requires type_in_bitset_v<T>
	static constexpr size_t index_v = index_of_v<T, TList<Ts...>>;

public:
	constexpr TypeIndexedBitMap() = default;
	explicit constexpr TypeIndexedBitMap(const ValT& initVal)
	{
		values_.fill(initVal);
	}

	template <typename T> requires type_in_bitset_v<T>
	void Set(bool tf) { bitset_.set(index_v<T>, tf); }

	template <typename T> requires type_in_bitset_v<T>
	constexpr bool Test() const { return bitset_.test(index_v<T>); }

	void Reset(const ValT& initVal) 
	{ 
		bitset_.reset(); 
		values_.fill(initVal);
	}

	template <typename T> requires (std::same_as<T, Ts> || ...)
	ValT& GetValue() { return values_[index_of_v<T, TList<Ts...>>]; }

	template <typename T> requires (std::same_as<T, Ts> || ...)
	const ValT& GetValue() const { return values_[index_of_v<T, TList<Ts...>>]; }

	template <typename Fn> requires std::invocable<Fn, bool&, ValT&>
	void ForEach(Fn&& fn)
	{
		for (size_t i = 0; i < values_.size(); ++i)
		{
			bool b = bitset_.test(i);

			std::invoke(fn, b, values_[i]);

			bitset_.set(i, b);
		}
	}

	template <typename Fn> requires std::invocable<Fn, bool, const ValT&>
	void ForEach(Fn&& fn) const
	{
		for (size_t i = 0; i < values_.size(); ++i)
		{
			std::invoke(fn, bitset_.test(i), values_[i]);
		}
	}
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
	void Set(bool tf) { (bitset_.set(static_cast<size_t>(event_traits<Ts>::index), tf) && ...); }

	void Set(uint32_t eventType, bool tf) { bitset_.set(static_cast<size_t>(eventType), tf); }

	template <SomeEventData...Ts> requires (sizeof...(Ts) > 0)
	bool Test() const { return (bitset_.test(static_cast<size_t>(event_traits<Ts>::index)) && ...); }

	bool Test(uint32_t eventType) const { return bitset_.test(static_cast<size_t>(eventType)); }

	template <SomeEventData...Ts> requires (sizeof...(Ts) > 0)
	bool TestAny() const { return (bitset_.test(static_cast<size_t>(event_traits<Ts>::index)) || ...); }

	const BitsetType& GetBitset() const { return bitset_; }

	bool operator==(const EventDataBitset&) const = default;

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
			bitset_ |= (component_traits<Ts>::bit | ...);
		}
		else
		{
			bitset_ &= ~(component_traits<Ts>::bit | ...);
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
		constexpr ComponentSignature sig = (component_traits<Ts>::bit | ...);
		return (bitset_ & sig) == sig;
	}

	constexpr bool Test(ComponentSignature sig) const { return (bitset_ & sig) == sig; }

	template <SomeComponent...Ts> requires (sizeof...(Ts) > 0)
	constexpr bool TestAny() const { return (bitset_ & (component_traits<Ts>::bit | ...)) != 0; }

	constexpr bool TestAny(ComponentSignature sig) const { return (bitset_ & sig) != 0; }

	operator BitsetType() const { return bitset_; }
	BitsetType GetBitset() const { return bitset_; }

	constexpr bool operator==(const ComponentBitset&) const = default;

private:
	BitsetType bitset_ = 0;
};
