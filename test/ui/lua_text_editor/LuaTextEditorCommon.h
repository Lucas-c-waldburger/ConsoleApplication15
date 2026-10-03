#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <string_view>

namespace ui {

static constexpr std::string_view kFunctionTableScriptTemplateText = R"(
	local table = {}

	#functions...

	local meta = {
		__signatures = { 

		}
	}
	
	setmetatable(table, meta)
			
	return table
)";

static constexpr std::string_view kSystemTableScriptTemplateText = R"(
	local table = {}

	#init
	function table.init()

	end

	#update
	function table.update(dt)

	end

	local meta = {
		__signatures = { 
			init = {},
			update = {"number"}
		}
	}
	
	setmetatable(table, meta)
			
	return table
)";



} // ui

#endif