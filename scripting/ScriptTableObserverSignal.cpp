#include "ScriptTableObserverSignal.h"

ScriptTableObserverPassKey ScriptTableObserver::GetScriptTableObserverPassKey() const
{
    return ScriptTableObserverPassKey::Get();
}

void ScriptTableRemovedNotifier::NotifyScriptTableRemoved(ScriptTable::TableId tableId)
{
    signal_.Emit(tableId);
}
