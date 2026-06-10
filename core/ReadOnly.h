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
    ~ReadOnly() = default;

    explicit ReadOnly(const T& data) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : data_(data) {}

    explicit ReadOnly(T&& data) noexcept(std::is_nothrow_move_constructible_v<T>)
        : data_(std::move(data)) {}

    ReadOnly(const ReadOnly& rhs) : data_(rhs.data_) {}
    ReadOnly(ReadOnly&& rhs) noexcept : data_(std::move(rhs.data_)) {}
    ReadOnly& operator=(const ReadOnly& rhs) {
        if (this != &rhs) {
            data_ = rhs.data_;
        }
        return *this;
    }
    ReadOnly& operator=(ReadOnly&& rhs) noexcept {
        if (this != &rhs) {
            data_ = std::move(rhs.data_);
        }
        return *this;
    }

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

    bool operator==(const ReadOnly& rhs) const requires std::equality_comparable<T>
    {
        return data_ == rhs.data_;
    }

    const T& GetData() const noexcept { return data_; }

    template <SomeWriteAccessorKey Key>
    T& GetDataMutable(Key&&) noexcept { return data_; }

private:
    T data_;
};

namespace std {
    template <typename T>
    struct hash<ReadOnly<T>> {
        size_t operator()(const ReadOnly<T>& ro) const noexcept /*requires Hashable<T>*/ {
            return std::hash<T>{}(ro.GetData());
        }
    };
}

namespace detail {
template <typename T> struct is_read_only_wrapped : std::false_type {};
template <typename T> struct is_read_only_wrapped<ReadOnly<T>> : std::true_type {};
}

template <typename T>
inline constexpr bool is_read_only_wrapped_v = detail::is_read_only_wrapped<T>::value;

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

template <typename Derived, typename...Ts>
class HasWriteAccessImpl : HasWriteAccess<Derived, Ts>...
{
protected:
    template <typename U> requires (std::same_as<U, Ts> || ...)
    U& GetWriteAccess(ReadOnly<U>& readOnly) const
    {
        return HasWriteAccess<Derived, U>::GetWriteAccess(readOnly);
    }
};


template <typename T>
class WriteAccessor : public HasWriteAccess<WriteAccessor<T>, T>
{
public:
    T& operator()(ReadOnly<T>& ro)
    {
        return this->GetWriteAccess(ro);
    }
};