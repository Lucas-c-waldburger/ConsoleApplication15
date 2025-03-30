#pragma once
#include "BaseComponent.h"
#include "../scripting/ScriptInfo.h"
#include "../core/TypeDomain.h"

struct Script : public BaseComponent<Script, 10>
{
    ScriptInfo activeScript;
    TypeDomain systemDomain;
};