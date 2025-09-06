#pragma once
#include <limits>
#include <type_traits>
#include <functional> 

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

