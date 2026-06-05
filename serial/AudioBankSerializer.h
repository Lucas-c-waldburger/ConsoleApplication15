#pragma once
#include "EntitySerializer.h"
#include "EntityDeserializer.h"

class AudioBankSerializer
{
public:
	static Result<Void> Serialize(const std::string& jsonFilepath, const AudioBank& bank);
	static std::vector<Error> Deserialize(const std::string& jsonFilepath, AudioBank& bank);

private:
	AudioBankSerializer() = default;
};