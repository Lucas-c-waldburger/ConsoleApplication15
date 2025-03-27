#include "Scripting.h"

Result<Void> Lua::Run()
{
	sol::protected_function_result ret;

	switch (scriptInfo_.scriptType)
	{
	case ScriptType::String:
		ret = lua_.script(scriptInfo_.name, sol::script_pass_on_error);
		break;
	case ScriptType::File:
		ret = lua_.script_file(scriptInfo_.name, sol::script_pass_on_error);
		break;
	case ScriptType::Unknown: default:
		return MAKE_ERROR("No script type specified");
	}

	if (!ret.valid())
	{
		sol::error err = ret;

		return MAKE_ERROR(std::string{err.what()});
	}
	return Void{};
}