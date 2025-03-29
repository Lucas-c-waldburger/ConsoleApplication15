#pragma once
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include "Handle.h"
#include "TypeUtils.h"


template <typename...HandleTs> //requires pack_types_unique_v<HandleTs...>
class HandleFactory
{
public:
    template <PackMemberType<HandleTs...> T>
    Handle<T> GetHandle()
    {
        return Handle<T>{
            Handle<T>::idCount++,
            Handle<T>::genCount
        };
    }

    template <PackMemberType<HandleTs...> T>
    void RetireHandle(Handle<T>& handle)
    {
        handle.id_ = -1;
        handle.gen_ = -1;
    }

    template <PackMemberType<HandleTs...> T>
    bool IsHandleValid(const Handle<T>& handle) const
    {
        return handle.id_ > -1 && handle.id_ < Handle<T>::idCount &&
               handle.gen_ == Handle<T>::genCount;

    }

    template <PackMemberType<HandleTs...> T>
    void InvalidateActiveHandles()
    {
        ++Handle<T>::genCount;
    }

private:
};