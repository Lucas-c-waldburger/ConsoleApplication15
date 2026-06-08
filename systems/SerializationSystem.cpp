#include "SerializationSystem.h"
#include "../serial/TextureRepositorySerializer.h"
#include "../serial/AudioBankSerializer.h"
#include "../serial/EntitySerializer.h"
#include "../serial/EntityDeserializer.h"
#include <ranges>

Result<Void>
SerializationSystem::SerializeState(const Filepaths& filepaths, const TextureRepository& textureRepo,
									const AudioBank& audioBank)
{
	TRY(TextureRepositorySerializer::Serialize(filepaths.texturesPath, textureRepo));
	TRY(AudioBankSerializer::Serialize(filepaths.audioPath, audioBank));

	auto entSerializer = EntitySerializer{ textureRepo, audioBank };
	TRY(entSerializer.SerializeEntities(filepaths.entitiesPath));

	return kVoid;
}

std::vector<Error>
SerializationSystem::DeserializeState(const Filepaths& filepaths, B2World& world,
									  TextureRepository& textureRepo, SDLInputSystem& inputSystem,
									  AudioBank& audioBank, SDL_Renderer* renderer)
{
	auto textureErrors = TextureRepositorySerializer::Deserialize(
		filepaths.texturesPath, textureRepo, renderer);
	auto audioErrors = AudioBankSerializer::Deserialize(
		filepaths.audioPath, audioBank);
	auto entityErrors = EntityDeserializer{ world, textureRepo, inputSystem, audioBank }
		.DeserializeEntities(filepaths.entitiesPath);

	const size_t totalErrorCount = textureErrors.size() + audioErrors.size() + entityErrors.size();
	if (totalErrorCount == 0)
	{
		return {};
	}

	std::vector<Error> allErrors{};
	allErrors.reserve(totalErrorCount);
	allErrors.append_range(std::move(textureErrors));
	allErrors.append_range(std::move(audioErrors));
	allErrors.append_range(std::move(entityErrors));

	return allErrors;
}