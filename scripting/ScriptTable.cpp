#include "ScriptTable.h"

ScriptTableView ScriptTable::GetView() const
{
    return ScriptTableView{ *this };
}
