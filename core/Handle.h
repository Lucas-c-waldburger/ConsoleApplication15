#pragma once
#include <unordered_map>
#include "CommonFunctions.h"

template <typename HandleDerived>
class IHandle;

template <template <typename> class HandleDerived, typename T>
class IHandle<HandleDerived<T>>
{
public: 
    using Super = IHandle<HandleDerived<T>>;

    IHandle() = default;

    bool IsValid() const { return static_cast<const HandleDerived<T>*>(this)->IsValidImpl(); }
    size_t GetHash() const noexcept { return static_cast<const HandleDerived<T>*>(this)->GetHashImpl(); }

    template <typename...Args>
    static HandleDerived<T> Create(Args&&...args) 
    { 
        return HandleDerived<T>::CreateImpl(std::forward<Args>(args)...); 
    }
};

template <typename T>
class Handle : public IHandle<Handle<T>>
{
public:   
    friend class IHandle<Handle<T>>;

    Handle() = default;
    constexpr bool operator==(const Handle& rhs) const noexcept
    {
        return id_ == rhs.id_; 
    }
    constexpr bool operator<(const Handle& rhs) const noexcept
    {
        return id_ < rhs.id_;
    }

private:
    size_t GetHashImpl() const noexcept
    {
        return std::hash<int>{}(id_);
    }

    bool IsValidImpl() const
    {
        return id_ < idCount;
    }

    static Handle CreateImpl()
    {
        return Handle{ idCount++ };
    }

    static inline int idCount = 0;

    Handle(int id) : id_(id) {}

    int id_ = -1;
};

namespace std {
    template <typename T>
    struct hash<Handle<T>> {
        size_t operator()(const Handle<T>& handle) const noexcept {
            return handle.GetHash();
        }
    };
}