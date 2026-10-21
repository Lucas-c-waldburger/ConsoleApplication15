#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <string_view>
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"

namespace ui {

static constexpr std::string_view kFunctionTableScriptTemplateText = R"(
local table = {}

# -- table functions -- #


# --------------------- #

local meta = {
    __signatures = { 

    }
}

setmetatable(table, meta)
			
return table
)";

static constexpr std::string_view kSystemTableScriptTemplateText = R"(
local table = {}

function table.init()

end

function table.update(dt)

end

local meta = {
    __signatures = { 
        init = {},
        update = {"number"}
    },
    __phase = Phase.Input
}
	
setmetatable(table, meta)
			
return table
)";

} // ui

#endif