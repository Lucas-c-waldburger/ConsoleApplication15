#pragma once
#include <memory>
#include <concepts>

using UniqueVoidPtr = std::unique_ptr<void, void(*)(void*)>;

template <typename T, typename...Args> requires std::constructible_from<T, Args...>
inline UniqueVoidPtr MakeUniqueVoidPtr(Args&&...args) {
	return UniqueVoidPtr(new T{ std::forward<Args>(args)... },
		[](void* p) { delete static_cast<T*>(p); });
}