#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED 
#include "FileTreeView.h"
#include "../../systems/ScriptSystem.h"

namespace ui {

class ScriptPicker
{
public:
    bool Draw(ScriptSystem& scriptSys)
    {
        auto package = scriptSys.ExportScriptDataPackage();
    }

private:

};



} // ui

#endif