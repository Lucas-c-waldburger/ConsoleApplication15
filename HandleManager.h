#pragma once
#include <iostream>
#include <unordered_set>
#include "Core.h"
#include "core/Handle.h"


template <typename T>
class HandleManagerTemplate;

template <typename...Ts>
class HandleManagerTemplate<TypeList<Ts...>>
{
public:
    HandleManagerTemplate() = default;
    ~HandleManagerTemplate() { NextGenAll(); }

    template <typename T>
    Handle<T> GetHandle()
    {
        auto& entry = GetEntry<T>();

        auto [handleIt, inserted] = entry.insert(Handle<T>::Create());
        assert(inserted);

        return *handleIt;
    }

    template <typename T>
    bool RetireHandle(Handle<T>& handle)
    {
        auto& entry = GetEntry<T>();

        bool wasValid = entry.erase(handle) > 0;

        handle = kInvalidHandle<T>;

        return wasValid;
    }

    template <typename T>
    void ToNextGeneration()
    {
        auto& entry = GetEntry<T>();

        std::erase_if(entry, IsCurrentGen<T>);

        Handle<T>::Regen();
    }

    template <typename T>
    bool IsValid(const Handle<T>& handle) const
    {
        if (handle == kInvalidHandle<T>) { return false; }

        const auto& entry = GetEntry<T>();

        return entry.contains(handle);
    }

private:
    template <typename T>
    std::unordered_set<Handle<T>>& GetEntry()
    {
        return std::get<std::unordered_set<Handle<T>>>(handles_);
    }

    template <typename T>
    const std::unordered_set<Handle<T>>& GetEntry() const
    {
        return std::get<std::unordered_set<Handle<T>>>(handles_);
    }

    template <typename T>
    static bool IsCurrentGen(const Handle<T>& handle)
    {
        return handle.gen_ == Handle<T>::genCounter;
    }

    void NextGenAll()
    {
        ((++Handle<Ts>::genCount), ...);
    }

    std::tuple<std::unordered_set<Handle<Ts>>...> handles_;
};

