#pragma once
#include "../core/Handle.h"
#include "../core/TypeUtils.h"

namespace detail {
template <typename T>
struct is_result : std::false_type {};

template <typename T>
struct is_result<Result<T>> : std::true_type {};
} // detail

template <typename T>
inline constexpr bool is_result_v = detail::is_result<T>::value;

template <typename T>
using add_result_t = 
	std::conditional_t<
		is_result_v<T>, 
		T, 
		std::conditional_t<
			std::is_void_v<T>,
			Result<Void>,
			Result<T>
		>
	>;




