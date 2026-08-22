#include "ScriptSystemSerializer.h"
#include "SerializationUtils.h"
#include "user_types/ScriptJsonUserTypes.h"

Result<Void> ScriptSystemSerializer::Serialize(const std::string& jsonFilepath, 
											   const ScriptSystem& scriptSys)
{
	std::ofstream file(jsonFilepath);
	if (!file)
	{
		return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'", jsonFilepath);
	}

	nlohmann::json j;

	auto& scriptJ = j["scripts"] = nlohmann::json::array();

	//auto descriptors = scriptSys.ExportTableDescriptors();
	//for (const auto& path : descriptors.filepaths)
	//{
	//	scriptJ.push_back(path);
	//}

	file << j.dump(4);

	return kVoid;
}

std::vector<Error> ScriptSystemSerializer::Deserialize(const std::string& jsonFilepath,
													   ScriptSystem& scriptSys)
{
	auto jResult = LoadJson(jsonFilepath);
	if (!jResult.Success())
	{
		return { jResult.GetError() };
	}
	const auto& j = jResult.GetValue();

	if (!j.contains("scripts"))
	{
		return { MAKE_ERROR("JSON file does not contain 'scripts' field") };
	}
	const auto& scriptsJ = j.at("scripts");

	if (!scriptsJ.is_array())
	{
		return { MAKE_ERROR("'scripts' field in JSON file was not of type array") };
	}

	ScriptTableDescriptors descriptors;
	descriptors.filepaths.reserve(scriptsJ.size());

	try
	{
		from_json(scriptsJ, descriptors.filepaths);
	}
	catch (const nlohmann::json::exception& err)
	{
		return { MAKE_ERROR_FMT("Error parsing script table descriptors: '{}'", err.what()) };
	}

	std::vector<Error> errors{};

	for (auto&& path : descriptors.filepaths)
	{
		auto addResult = scriptSys.AddTable(path);
		if (!addResult.Success())
		{
			errors.emplace_back(std::move(addResult.GetError()));
			continue;
		}
	}

	return errors;
}

void ScriptSystemSerializer::SerializeToJson(nlohmann::json& masterJ, 
											 const ScriptSystem& scriptSys)
{
	assert(!masterJ.contains("scripts"));

	auto& scriptJ = masterJ["scripts"] = nlohmann::json::array();

	//auto descriptors = scriptSys.ExportTableDescriptors();
	//for (const auto& path : descriptors.filepaths)
	//{
	//	scriptJ.push_back(path);
	//}
}

std::vector<Error> ScriptSystemSerializer::DeserializeFromJson(const nlohmann::json& masterJ, 
															   ScriptSystem& scriptSys)
{
	if (!masterJ.contains("scripts"))
	{
		return { MAKE_ERROR("JSON file does not contain 'scripts' field") };
	}
	const auto& scriptsJ = masterJ.at("scripts");

	if (!scriptsJ.is_array())
	{
		return { MAKE_ERROR("'scripts' field in JSON file was not of type array") };
	}

	ScriptTableDescriptors descriptors;
	descriptors.filepaths.reserve(scriptsJ.size());

	try
	{
		from_json(scriptsJ, descriptors.filepaths);
	}
	catch (const nlohmann::json::exception& err)
	{
		return { MAKE_ERROR_FMT("Error parsing script table descriptors: '{}'", err.what()) };
	}

	std::vector<Error> errors{};

	for (auto&& path : descriptors.filepaths)
	{
		auto addResult = scriptSys.AddTable(path);
		if (!addResult.Success())
		{
			errors.emplace_back(std::move(addResult.GetError()));
			continue;
		}
	}

	return errors;
}
