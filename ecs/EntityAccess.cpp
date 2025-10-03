#include "EntityAccess.h"

EntityPassKey EntityFullAccessPrivelage::GetEntityPassKey() const
{
    return EntityPassKey::Get();
}
