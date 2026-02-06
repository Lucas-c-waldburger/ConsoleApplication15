#pragma once
#include <concepts>

template <size_t StorageSize>
class InlineStorage
{
private:
    struct Ops
    {
        void(*destroy)(void*) = nullptr;
        void(*move)(void*, void*) = nullptr;
        void(*copy)(void*, const void*) = nullptr;
    };

    alignas(std::max_align_t) unsigned char storage[StorageSize]{};
    Ops ops{};

public:
    InlineStorage() = default;
    ~InlineStorage() { Reset(); }

    // Prevent copy/move if not available
    InlineStorage(const InlineStorage& other)
    {
        if (other.ops.copy)
        {
            other.ops.copy(storage, other.storage);
        }
        ops = other.ops;
    }

    InlineStorage(InlineStorage&& other) noexcept
    {
        if (other.ops.move)
        {
            other.ops.move(storage, other.storage);
        }

        ops = other.ops;
        other.ops = Ops{};
    }

    InlineStorage& operator=(const InlineStorage& other)
    {
        Reset();
        if (other.ops.copy)
        {
            other.ops.copy(storage, other.storage);
        }

        ops = other.ops;

        return *this;
    }

    InlineStorage& operator=(InlineStorage&& other) noexcept
    {
        Reset();
        if (other.ops.move)
        {
            other.ops.move(storage, other.storage);
        }
        ops = other.ops;
        other.ops = Ops{};

        return *this;
    }

    template <typename T, typename... Args>
        requires(sizeof(T) <= StorageSize && std::is_nothrow_move_constructible_v<T>)
    T& Emplace(Args&&...args)
    {
        Reset();
        new(storage) T(std::forward<Args>(args)...);

        ops.destroy = [](void* ptr) {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                reinterpret_cast<T*>(ptr)->~T();
            }
            };

        ops.move = [](void* dst, void* src) {
            new(dst) T(std::move(*reinterpret_cast<T*>(src)));
            reinterpret_cast<T*>(src)->~T();
        };

        if constexpr (std::is_copy_constructible_v<T>)
        {
            ops.copy = [](void* dst, const void* src) {
                new(dst) T(*reinterpret_cast<const T*>(src));
            };
        }

        return *reinterpret_cast<T*>(storage);
    }

    void Reset()
    {
        if (ops.destroy)
        {
            ops.destroy(storage);
        }
        ops = Ops{};
    }

    template <typename T>
    T& Get()
    {
        return *reinterpret_cast<T*>(storage);
    }

    template <typename T>
    const T& Get() const
    {
        return *reinterpret_cast<const T*>(storage);
    }

    bool HasValue() const { return ops.destroy != nullptr; }
    bool operator!() const { return !HasValue(); }

    // Swappable
    friend void swap(InlineStorage& lhs, InlineStorage& rhs) noexcept
    {
        alignas(std::max_align_t) unsigned char temp[StorageSize]{};

        if (lhs.HasValue())
        {
            lhs.ops.move(temp, lhs.storage);
        }
        if (rhs.HasValue())
        {
            rhs.ops.move(lhs.storage, rhs.storage);
        }
        if (lhs.HasValue())
        {
            lhs.ops.move(rhs.storage, temp);
        }

        using std::swap;
        swap(lhs.ops, rhs.ops);
    }
};