#pragma once
#include "EntitySerializer.h"
#include "EntityDeserializer.h"

class TextureRepositorySerializer
{
public:
	static Result<Void> Serialize(const std::string& jsonFilepath, const TextureRepository& repo);
	static std::vector<Error> Deserialize(const std::string& jsonFilepath, TextureRepository& repo, 
										  SDL_Renderer* renderer);

	static void SerializeToJson(nlohmann::json& masterJ, const TextureRepository& repo);
	static std::vector<Error> DeserializeFromJson(const nlohmann::json& masterJ, TextureRepository& repo,
												  SDL_Renderer* renderer);

private:
	TextureRepositorySerializer() = default;
};