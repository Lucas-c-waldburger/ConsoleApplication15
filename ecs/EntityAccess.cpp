#include "EntityAccess.h"

EntityPassKey EntityFullAccessPrivelage::GetEntityPassKey()
{
    return EntityPassKey::Get();
}
