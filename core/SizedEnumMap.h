#pragma once
#include "SizedEnum.h"
#include <array>
#include <cassert>



template <SomeSizedEnum EnumKey, typename Value>
class SizedEnumMap
{
public:
	using MapType = std::array<Value, enum_size_v<EnumKey>>;

	template <typename MapRef>
	struct IteratorTemplate
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::pair<EnumKey, std::remove_reference_t<MapRef>&>;
		using difference_type = std::ptrdiff_t;
		using reference = std::pair<EnumKey, MapRef&>;

		constexpr IteratorTemplate(MapRef& map, size_t idx) : map_(map), idx_(idx) {}

		constexpr reference operator*() const {
			return { static_cast<EnumKey>(idx_), map_[idx_] };
		}

		constexpr IteratorTemplate& operator++() { ++idx_; return *this; }
		constexpr IteratorTemplate operator++(int) { IteratorTemplate tmp = *this; ++idx_; return tmp; }

		constexpr bool operator==(const IteratorTemplate& other) const { return idx_ == other.idx_; }
		constexpr bool operator!=(const IteratorTemplate& other) const { return idx_ != other.idx_; }

	private:
		MapRef& map_;
		size_t idx_;
	};

	using Iterator = IteratorTemplate<Value>;
	using ConstIterator = IteratorTemplate<const Value>;

	constexpr Iterator begin() { return Iterator(map_, 0); }
	constexpr Iterator end() { return Iterator(map_, map_.size()); }

	constexpr ConstIterator begin() const { return ConstIterator(map_, 0); }
	constexpr ConstIterator end()   const { return ConstIterator(map_, map_.size()); }

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
		auto idx = static_cast<std::underlying_type_t<EnumKey>>(key);

		//assert(idx >= 0 && idx < enum_size_v<EnumKey>);

		return map_[static_cast<size_t>(idx)];
	}

	constexpr const auto& operator[](EnumKey key) const
	{
		auto idx = static_cast<std::underlying_type_t<EnumKey>>(key);

		//assert(idx >= 0 && idx < enum_size_v<EnumKey>);

		return map_[static_cast<size_t>(idx)];
	}

	constexpr auto& operator[](size_t idx)
	{
		return map_[idx];
	}

	constexpr const auto& operator[](size_t idx) const
	{
		return map_[idx];
	}

	constexpr size_t Size() const { return enum_size_v<EnumKey>; }

	//constexpr auto begin() { return map_.begin(); }
	//constexpr auto begin() const { return map_.begin(); }

	//constexpr auto end() { return map_.end(); }
	//constexpr auto end() const { return map_.end(); }

private:
	MapType map_;
};