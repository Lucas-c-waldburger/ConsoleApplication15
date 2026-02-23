#pragma once
#include <limits>
#include <type_traits>
#include <functional> 
#include <vector>

template <typename Family, typename T>
struct TypeInFamily : std::false_type {};

template <typename Family>
struct FamilyTypeID
{
private:
	static inline size_t counter_ = 0;

public:
	template <typename T> 
		requires (std::same_as<T, raw_type_t<T>> && TypeInFamily<Family, T>::value)
	static inline const size_t value = counter_++;
};


//template <typename T, int = 0> 
//class SparseTypeSet
//{
//private:
//	using TypeID = uint32_t;
//
//	static TypeID NextTypeID()
//	{
//		static TypeID id = 0;
//		return id++;
//	}
//
//	template <typename U>
//	static TypeID GetTypeID()
//	{
//		static TypeID id = NextTypeID();
//		return id;
//	}
//
//public:
//	template <typename U, typename...Args>
//		requires std::constructible_from<U, Args...>
//	bool Insert(Args&&...args)
//	{
//		const auto id = static_cast<size_t>(NextTypeID<U>());
//		if (id >= typeIdToIndex_.size())
//		{
//			typeIdToIndex_.resize(id + 1, std::numeric_limits<size_t>::max());
//		}
//		if (typeIdToIndex_[id] == std::numeric_limits<size_t>::max())
//		{
//			typeIdToIndex_[id] = data_.size();
//			data_.emplace_back(std::forward<Args>(args)...);
//
//			return true;
//		}
//
//		return false;		
//	}
//
//	template <typename U>
//	T& At()
//	{
//		const auto id = static_cast<size_t>(NextTypeID<U>());
//		assert(id < typeIdToIndex_.size());
//		assert(typeIdToIndex_[id] < data_.size());
//
//		return data_[typeIdToIndex_[id]];
//	}
//
//	template <typename U>
//	bool Erase()
//	{
//		if (data_.empty())
//		{
//			return false;
//		}
//
//		const auto id = static_cast<size_t>(NextTypeID<U>());
//		if (id >= typeIdToIndex_.size() ||
//			typeIdToIndex_[id] == std::numeric_limits<size_t>::max())
//		{
//			return false;
//		}
//
//		const size_t idxToErase = typeIdToIndex_[id];
//		const size_t backIdx = data_.size() - 1;
//		assert(idxToErase < data_.size());
//
//		if (idxToErase != backIdx)
//		{
//			auto backDataTypeIdSlot = core::Find(typeIdToIndex_, backIdx);
//			assert(backDataTypeIdSlot != typeIdToIndex_.end());
//
//			*backDataTypeId = idxToErase;
//			typeIdToIndex_[id] = std::numeric_limits<size_t>::max();
//
//			std::swap(data_[idxToErase], data_[backIdx]);
//		}
//
//		data_.pop_back();
//
//		return true;
//	}
//
//	template <typename U>
//	bool Contains() const
//	{
//		const auto id = static_cast<size_t>(NextTypeID<U>());
//
//		return id < typeIdToIndex_.size() &&
//			   typeIdToIndex_[id] == std::numeric_limits<size_t>::max();
//	}
//
//private:
//	std::vector<size_t> typeIdToIndex_;
//	std::vector<T> data_;
//};