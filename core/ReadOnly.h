#pragma once
#include <concepts>

// wraps T inside a const-only getter
template <typename T> requires std::default_initializable<T>
class ReadOnly
{
public:
    ReadOnly() = default;

    explicit ReadOnly(const T& data) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : data_(data) {}

    explicit ReadOnly(T&& data) noexcept(std::is_nothrow_move_constructible_v<T>)
        : data_(std::move(data)) {}

    ReadOnly& operator=(const T& data) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        data_ = data;
        return *this;
    }

    ReadOnly& operator=(T&& data) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        data_ = std::move(data);
        return *this;
    }

    const T& GetData() const noexcept { return data_; }

private:
    T data_;
};
