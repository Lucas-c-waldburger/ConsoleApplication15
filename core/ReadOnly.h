#pragma once
#include <concepts>

// mutable accessor
template <typename Derived, typename ReadOnlyType>
class HasWriteAccess;

template <typename T, typename U>
concept SomeWriteAccessor = std::derived_from<T, HasWriteAccess<T, U>>;

template <typename T>
concept SomeWriteAccessorKey = std::same_as<T,
    typename HasWriteAccess<typename T::DerivedType, typename T::ValueType>::Key>;

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

    template <SomeWriteAccessorKey Key>
    T& GetDataMutable(Key&&) noexcept { return data_; }

private:
    T data_;
};


template <typename Derived, typename T>
class HasWriteAccess
{
public:
    using DataType = T;

private:
    struct Key
    {
        using DerivedType = Derived;
        using ValueType = T;
    };

protected:
    T& GetWriteAccess(ReadOnly<T>& readOnly) const
    {
        return readOnly.GetDataMutable(Key{});
    }
};