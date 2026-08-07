#pragma once
#include "EntitySerializer.h"
#include "EntityDeserializer.h"
#include "../systems/ScriptSystem.h"
#include "../deps/nlohmann/json.hpp"

class ScriptSystemSerializer
{
public:
	static Result<Void> Serialize(const std::string& jsonFilepath, const ScriptSystem& scriptSys);
	static std::vector<Error> Deserialize(const std::string& jsonFilepath, ScriptSystem& scriptSys);

	static void SerializeToJson(nlohmann::json& masterJ, const ScriptSystem& scriptSys);
	static std::vector<Error> DeserializeFromJson(const nlohmann::json& masterJ, ScriptSystem& scriptSys);

private:
	ScriptSystemSerializer() = default;
};