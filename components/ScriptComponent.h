#pragma once
#include "BaseComponent.h"
#include "../scripting/ScriptInfo.h"
#include "../core/TypeDomain.h"

struct Script : public BaseComponent<Script, 11>
{
    ScriptInfo activeScript;
    TypeDomain systemDomain;
};