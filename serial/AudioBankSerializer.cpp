#include "AudioBankSerializer.h"
#include "SerializationUtils.h"

Result<Void> AudioBankSerializer::Serialize(const std::string& jsonFilepath, const AudioBank& bank)
{
	std::ofstream file(jsonFilepath);
	if (!file)
	{
		return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'", jsonFilepath);
	}

	nlohmann::json j;

	auto& audioJ = j["audio"] = nlohmann::json::array();

	auto audioPackage = bank.ExportAudioDescriptors();
	for (const auto& descriptor : audioPackage)
	{
		audioJ.push_back(descriptor);
	}

	file << j.dump(4);

	return kVoid;
}

std::vector<Error> AudioBankSerializer::Deserialize(const std::string& jsonFilepath, AudioBank& bank)
{
	auto jResult = LoadJson(jsonFilepath);
	if (!jResult.Success())
	{
		return { jResult.GetError() };
	}
	const auto& j = jResult.GetValue();

	if (!j.contains("audio"))
	{
		return { MAKE_ERROR("JSON file does not contain 'audio' field") };
	}
	const auto& audioJ = j.at("audio");

	if (!audioJ.is_array())
	{
		return { MAKE_ERROR("'audio' field in JSON file was not of type array") };
	}

	std::vector<AudioDescriptor> audioDescriptors;
	audioDescriptors.reserve(audioJ.size());

	try 
	{
		from_json(audioJ, audioDescriptors);
	}
	catch (const nlohmann::json::exception& err)
	{
		return { MAKE_ERROR_FMT("Error parsing audio descriptors: '{}'", err.what()) };
	}

	std::vector<Error> errors{};

	for (auto&& descriptor : audioDescriptors)
	{
		auto loadResult = bank.LoadAudio(std::move(descriptor));
		if (!loadResult.Success())
		{
			errors.emplace_back(std::move(loadResult.GetError()));
		}
	}

	return errors;
}

void AudioBankSerializer::SerializeToJson(nlohmann::json& masterJ, const AudioBank& bank)
{
	assert(!masterJ.contains("audio"));

	auto& audioJ = masterJ["audio"] = nlohmann::json::array();

	auto audioPackage = bank.ExportAudioDescriptors();
	for (const auto& descriptor : audioPackage)
	{
		audioJ.push_back(descriptor);
	}
}

std::vector<Error> AudioBankSerializer::DeserializeFromJson(const nlohmann::json& masterJ, 
														    AudioBank& bank)
{
	if (!masterJ.contains("audio"))
	{
		return { MAKE_ERROR("JSON file does not contain 'audio' field") };
	}
	const auto& audioJ = masterJ.at("audio");

	if (!audioJ.is_array())
	{
		return { MAKE_ERROR("'audio' field in JSON file was not of type array") };
	}

	std::vector<AudioDescriptor> audioDescriptors;
	audioDescriptors.reserve(audioJ.size());

	try
	{
		from_json(audioJ, audioDescriptors);
	}
	catch (const nlohmann::json::exception& err)
	{
		return { MAKE_ERROR_FMT("Error parsing audio descriptors: '{}'", err.what()) };
	}

	std::vector<Error> errors{};

	for (auto&& descriptor : audioDescriptors)
	{
		auto loadResult = bank.LoadAudio(std::move(descriptor));
		if (!loadResult.Success())
		{
			errors.emplace_back(std::move(loadResult.GetError()));
		}
	}

	return errors;
}
