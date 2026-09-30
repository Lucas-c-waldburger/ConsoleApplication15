#pragma once
#include <cassert>
#include "../core/Handle.h"

class ScriptTable;

template <>
class Handle<ScriptTable> : public IHandle<Handle<ScriptTable>>
{
public:
    friend class Super;

    Handle() = default;
    constexpr bool operator==(const Handle<ScriptTable>& rhs) const noexcept
    {
        return tableId_ == rhs.tableId_ && resourceIndex_ == rhs.resourceIndex_;
    }

    // The script table the resource is sourced from
    constexpr uint32_t GetSourceID() const noexcept
    {
        return tableId_;
    }
    // The index of the resource within the info SOA
    constexpr uint32_t GetResourceIndex() const noexcept
    {
        return resourceIndex_;
    }

private:
    size_t GetHashImpl() const noexcept
    {
        return MakeHash(tableId_, resourceIndex_);
    }
    bool IsValidImpl() const
    {
        return tableId_ != std::numeric_limits<uint32_t>::max() &&
               resourceIndex_ != std::numeric_limits<uint32_t>::max();
    }

    static Handle CreateImpl(uint32_t tableId, size_t resourceIndex)
    {
        return Handle{ tableId, resourceIndex };
    }

    Handle(uint32_t tableId, size_t resourceIndex) :
        tableId_(tableId), resourceIndex_(resourceIndex)
    {
        assert(resourceIndex <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
    }

    uint32_t tableId_ = std::numeric_limits<uint32_t>::max();
    uint32_t resourceIndex_ = std::numeric_limits<uint32_t>::max();
};