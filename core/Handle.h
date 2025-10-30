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
    template <typename...Ts>
    friend class HandleFactory;

    Handle() = default;
    bool operator==(const Handle& rhs) const { return id_ == rhs.id_ && gen_ == rhs.gen_; }
    bool operator<(const Handle& rhs) const
    {
        return (gen_ != rhs.gen_) ? gen_ < rhs.gen_ : id_ < rhs.id_;
    }

private:
    size_t GetHashImpl() const noexcept
    {
        size_t hash = 0;
        HashCombine(hash, std::hash<int>{}(id_));
        HashCombine(hash, std::hash<int>{}(gen_));

        return hash;
    }

    bool IsValidImpl() const
    {
        return id_ < idCount && gen_ == genCount;
    }

    static Handle CreateImpl()
    {
        return Handle{ idCount++, genCount };
    }

    static inline int idCount = 0;
    static inline int genCount = 0;

    Handle(int id, int gen) : id_(id), gen_(gen) {}

    int id_ = -1;
    int gen_ = -1;
};

namespace std {
    template <typename T>
    struct hash<Handle<T>> {
        size_t operator()(const Handle<T>& handle) const noexcept {
            return handle.GetHash();
        }
    };
}

//template <typename T>
//class Handle
//{
//public:
//    friend class HandleFactory;
//
//    template <typename>
//    friend class HandleManagerTemplate;
//
//    Handle() = default;
//
//    bool operator==(const Handle&) const = default;
//
//    size_t GetHash() const noexcept
//    {
//        return static_cast<size_t>(id_) * 31 + static_cast<size_t>(gen_);
//    }
//
//    friend std::ostream& operator<<(std::ostream& os, const Handle<T>& handle)
//    {
//        if (handle.id_ == -1 && handle.gen_ == -1)
//        {
//            os << "{ INVALID }";
//        }
//        else
//        {
//            os << "{ id: " << handle.id_ << ", gen: " << handle.gen_ << " }";
//        }
//
//        return os;
//    }
//
//private:
//    static inline int idCount = 0;
//    static inline int genCount = 0;
//
//    Handle(int id, int gen) : id_(id), gen_(gen) {}
//
//    static Handle Create() { return Handle{ idCount++, genCount }; }
//    static void Regen() { idCount = 0; ++genCount; }
//
//    int id_ = -1;
//    int gen_ = -1;
//};
//
////template <typename T>
////static const Handle<T> kInvalidHandle = Handle<T>{};
//
//namespace std {
//    template <typename T>
//    struct hash<Handle<T>> {
//        size_t operator()(const Handle<T>& handle) const noexcept {
//            return handle.GetHash();
//        }
//    };
//}
//
//// ALIASES
//template <typename T> using HandleMap = std::unordered_map<Handle<T>, T>;