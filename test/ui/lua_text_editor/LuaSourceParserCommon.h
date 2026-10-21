#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <string>

namespace ui {

struct SourceLocation
{
	size_t start = 0;
	size_t end = 0;
};

struct SourceDocumentError
{
	enum class Type
	{
		TableFunctionNotFoundInSignatures,
		SignatureFunctionNotFoundForTable,
		ParameterCountMismatch
	};

	std::string message;
	SourceLocation location;
};

} // ui

#endif