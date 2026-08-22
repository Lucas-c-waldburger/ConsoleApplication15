#pragma once
#include "../core/Signal.h"
#include "ScriptTable.h"

class ScriptTableObserverPassKey;

class ScriptTableObserver
{
protected:
    ScriptTableObserverPassKey GetScriptTableObserverPassKey() const;
};

class ScriptTableObserverPassKey
{
    friend class ScriptTableObserver;
    constexpr ScriptTableObserverPassKey() = default;
    static constexpr ScriptTableObserverPassKey Get() { return {}; }
};

using ScriptTableObserverSignal = 
    PrivateSignal<ScriptTableObserverPassKey, ScriptTable::TableId>;

class ScriptTableRemovedNotifier
{
public:
    template <typename Fn>
    SignalToken ConnectScriptTableObserver(ScriptTableObserverPassKey pk, Fn&& fn)
    {
        signal_ = {};

        return signal_.Connect(pk, std::forward<Fn>(fn));
    }

protected:
    void NotifyScriptTableRemoved(ScriptTable::TableId tableId);

private:
    ScriptTableObserverSignal signal_;
};