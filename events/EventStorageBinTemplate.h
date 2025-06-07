#pragma once
#include <vector>
#include "custom/CustomEventData.h"


template <SomeCustomEvent...Ts> requires pack_types_unique_v<Ts...>
class EventDataStorageBinTemplate
{
public:
    EventDataStorageBinTemplate() = default;

    template <PackMemberType<Ts...> T>
    const std::vector<T>& GetEntry() const
    {
        return std::get<std::vector<T>>(eventDatas_);
    }

    template <PackMemberType<Ts...> T>
    const T& Emplace(T&& data)
    {
        return std::get<std::vector<T>>(eventDatas_).emplace_back(std::forward<T>(data));
    }

    template <PackMemberType<Ts...> T>
    void ClearEntry()
    {
        std::get<std::vector<T>>(eventDatas_).clear();
    }

    void ClearAll()
    {
        ((ClearEntry<Ts>()), ...);
    }

private:
    std::tuple<std::vector<Ts>...> eventDatas_;
};