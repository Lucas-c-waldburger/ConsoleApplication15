#pragma once
#include "SizedEnum.h"
#include "Literals.h"
#include <array>
#include <cassert>



template <SomeSizedEnum EnumKey, typename Value>
class SizedEnumMap
{
public:
	using MapType = std::array<Value, enum_size_v<EnumKey>>;

	template <typename V>
	struct IteratorTemplate
	{
		using MapPtr = 
			std::conditional_t<std::is_const_v<V>, const MapType*, MapType*>;

		struct Item
		{
			constexpr Item(EnumKey k, V& v) : key(k), val(v) {}

			EnumKey key;
			V& val;

			friend constexpr bool operator==(const Item& lhs, const Item& rhs) {
				return lhs.key == rhs.key && lhs.val == rhs.val;
			}
		};

		using iterator_category = std::forward_iterator_tag;
		using value_type = Item;
		using difference_type = std::ptrdiff_t;
		using reference = Item&;
		using pointer = Item*;

		constexpr IteratorTemplate(MapPtr arr = nullptr, std::size_t idx = 0)
			: arr_(arr), idx_(idx) {}

		constexpr value_type operator*() const
		{
			return Item( static_cast<EnumKey>(idx_), (*arr_)[idx_] );
		}

		constexpr IteratorTemplate& operator++() 
		{ 
			++idx_;
			return *this; 
		}
		constexpr IteratorTemplate operator++(int) 
		{ 
			IteratorTemplate tmp = *this; 
			++(*this);
			return tmp; 
		}

		constexpr bool operator==(IteratorTemplate const& other) const 
		{ 
			return arr_ == other.arr_ && idx_ == other.idx_;
		}
		constexpr bool operator!=(IteratorTemplate const& other) const 
		{ 
			return !(*this == other);
		}



	private:
		MapPtr arr_;
		size_t idx_;
	};

	using iterator = IteratorTemplate<Value>;
	using const_iterator = IteratorTemplate<const Value>;

	constexpr iterator begin() { return iterator(&map_, 0_uz); }
	constexpr iterator end() { return iterator(&map_, map_.size()); }

	constexpr const_iterator begin() const { return const_iterator(&map_, 0_uz); }
	constexpr const_iterator end()   const { return const_iterator(&map_, map_.size()); }

	constexpr const_iterator cbegin() const { return begin(); }
	constexpr const_iterator cend() const { return end(); }

	template <typename K, typename V, typename...Rest>
		requires (std::same_as<std::remove_cvref_t<K>, EnumKey> && 
				  std::convertible_to<V, Value>)
	static constexpr void AddPairs(MapType& map, K k, V&& v, Rest&&...rest)
	{
		map[static_cast<size_t>(k)] = std::forward<V>(v);
		if constexpr (sizeof...(rest) > 0) 
		{
			AddPairs(map, std::forward<Rest>(rest)...);
		}
	}

	constexpr SizedEnumMap() = default;

	template <typename...Args> 
		requires (sizeof...(Args) > 0 && sizeof...(Args) % 2 == 0)
	constexpr SizedEnumMap(Args&&...args)
	{
		AddPairs(map_, std::forward<Args>(args)...);
	}

	//template <typename...Args> 
	//	requires (std::convertible_to<Args, std::pair<EnumKey, Value>> && ...)
	//constexpr SizedEnumMap(Args&&...args)
	//{
	//	constexpr auto impl = [](auto& map, auto&& pair) {
	//		using SecType = typename std::remove_cvref_t<decltype(pair)>::second_type;
	//		auto idx = static_cast<std::underlying_type_t<EnumKey>>(pair.first);
	//		map[idx] = std::forward<SecType>(pair.second);
	//	};

	//	((impl(map_, std::forward<Args>(args))), ...);
	//}

	template <typename Arg> requires std::convertible_to<Arg, MapType>
	explicit constexpr SizedEnumMap(Arg arg) : map_{ std::forward<Arg>(arg) } {}

	constexpr auto& operator[](EnumKey key)
	{
		return map_[static_cast<size_t>(key)];
	}

	constexpr const auto& operator[](EnumKey key) const
	{
		return map_[static_cast<size_t>(key)];
	}

	constexpr auto& operator[](size_t idx)
	{
		return map_[idx];
	}

	constexpr const auto& operator[](size_t idx) const
	{
		return map_[idx];
	}

	template <EnumKey e>
	constexpr auto& operator()() { return map_[static_cast<size_t>(e)]; }

	template <EnumKey e>
	constexpr const auto& operator()() const { return map_[static_cast<size_t>(e)]; }

	constexpr size_t Size() const { return enum_size_v<EnumKey>; }

private:
	MapType map_;
};