#pragma once
#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../deps/tinyfiledialogs/tinyfiledialogs.h"
#include "../../systems/ScriptSystem.h"
#include "../../file/FilePathUtility.h"

namespace ui {

class ScriptLoaderUtility
{
public:
	static Result<bool> HandleScriptSelection(ScriptSystem& scriptSys)
	{
		static constexpr const char* filters[] = { "*.lua" };

		const char* selectedFile = tinyfd_openFileDialog(
			"Load Script",
			GetDefaultScriptsFolderString().c_str(),
			1,
			filters,
			".lua",
			0
		);

		if (!selectedFile)
		{
			return false;
		}

		TRY(scriptSys.AddTable(selectedFile));

		return true;
	}

private:
	static const std::string& GetDefaultScriptsFolderString()
	{
		static const std::filesystem::path kDefaultScriptsFolderPath =
			FilePathUtility::GetRootPath() / "test" / "catch" / "test_scripts" / "";

		static const std::string kDefaultScriptsFolderString =
			kDefaultScriptsFolderPath.string();

		return kDefaultScriptsFolderString;
	}
};



} // ui

#endif