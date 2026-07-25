#pragma once
#include "EntitySerializer.h"
#include "EntityDeserializer.h"
#include "../deps/nlohmann/json.hpp"

class AudioBankSerializer
{
public:
	static Result<Void> Serialize(const std::string& jsonFilepath, const AudioBank& bank);
	static std::vector<Error> Deserialize(const std::string& jsonFilepath, AudioBank& bank);

	static void SerializeToJson(nlohmann::json& masterJ, const AudioBank& bank);
	static std::vector<Error> DeserializeFromJson(const nlohmann::json& masterJ, AudioBank& bank);

private:
	AudioBankSerializer() = default;
};