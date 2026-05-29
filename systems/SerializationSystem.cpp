#include "SerializationSystem.h"
#include "../serial/TextureRepositorySerializer.h"
#include "../serial/EntitySerializer.h"
#include "../serial/EntityDeserializer.h"
#include <ranges>

Result<Void>
SerializationSystem::SerializeState(const Filepaths& filepaths, const TextureRepository& textureRepo)
{
	TRY(TextureRepositorySerializer::Serialize(filepaths.texturesPath, textureRepo));
	TRY(EntitySerializer{ textureRepo }.SerializeEntities(filepaths.entitiesPath));

	return kVoid;
}

std::vector<Error>
SerializationSystem::DeserializeState(const Filepaths& filepaths, B2World& world,
									  TextureRepository& textureRepo, SDLInputSystem& inputSystem,
									  SDL_Renderer* renderer)
{
	auto repoErrors = TextureRepositorySerializer::Deserialize(
		filepaths.texturesPath, textureRepo, renderer);
	auto entityErrors = EntityDeserializer{ world, textureRepo, inputSystem }
		.DeserializeEntities(filepaths.entitiesPath);

	const size_t totalErrorCount = repoErrors.size() + entityErrors.size();
	if (totalErrorCount == 0)
	{
		return {};
	}

	std::vector<Error> allErrors{};
	allErrors.reserve(totalErrorCount);
	allErrors.append_range(std::move(repoErrors));
	allErrors.append_range(std::move(entityErrors));

	return allErrors;
}

//Result<Void> SerializationSystem::SerializeState(std::string_view jsonFilename, 
//												 TextureRepository& textureRepo)
//{
//	// serialize atlases
//	nlohmann::json j;
//	
//	auto& atlasesJson = j[kTextureAtlasesKey] = nlohmann::json::object();
//
//	TRY(util::TextureRepositorySerializationHelper::SerializeAtlases(
//		textureRepo, atlasesJson));
//
//	// serialize entities
//	auto entities = ECS::GetAllActiveEntities();
//	if (entities.empty())
//	{
//		return Void{};
//	}
//
//	auto pathStr = JoinPathsRaw(kJsonDirName, jsonFilename).string();
//
//	std::ofstream file(pathStr);
//	if (!file) 
//	{
//		return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'", pathStr);
//	}
//
//	auto& entityArr = j[kJsonEntitiesKey] = nlohmann::json::array();
//
//	for (const auto& entity : entities)
//	{
//		auto& entityJson = entityArr.emplace_back(entity);
//
//		TRY(util::CheckJsonKey(entityJson, kJsonComponentsKey));
//
//		auto resolveResult = util::ComponentSerializationResolver::Serialize(
//			entity, textureRepo, entityJson.at(kJsonComponentsKey)
//		);
//		if (!resolveResult.Success())
//		{
//			LOG_ERROR(resolveResult.GetError());
//		}
//	}
//
//	file << std::setw(4) << j;
//
//	return Void{};
//}
//
//std::vector<Error> SerializationSystem::DeserializeState(
//	std::string_view jsonFilename, TextureRepository& textureRepo)
//{
//	std::vector<Error> errors{};
//
//	// load json
//	auto path = ResourcePath::Json(jsonFilename);
//	if (!path.Success())
//	{
//		errors.push_back(path.GetError());
//		return errors;
//	}
//
//	std::ifstream file(path.GetValue());
//	if (!file)
//	{
//		errors.push_back(MAKE_ERROR_FMT("Could not open exisiting JSON "
//			" file at path: '{}'", path.GetValue()));
//		return errors;
//	}
//
//	nlohmann::json j;
//	try 
//	{
//		j = nlohmann::json::parse(file);
//	}
//	catch (const nlohmann::json::parse_error& err) 
//	{
//		errors.push_back(MAKE_ERROR_FMT("JSON parse error: '{}'", err.what()));
//		return errors;
//	}
//
//	// deserialize atlases
//	auto checkAtlases = util::CheckJsonKey(j, kTextureAtlasesKey, 
//										   nlohmann::json::value_t::object);
//	if (!checkAtlases.Success())
//	{
//		errors.push_back(std::move(checkAtlases.GetError()));
//		return errors;
//	}
//
//	auto handleHashMapResult =
//		util::TextureRepositorySerializationHelper::DeserializeAtlases(
//			SDLite::Renderer(), textureRepo, j.at(kTextureAtlasesKey));
//	if (!handleHashMapResult.Success())
//	{
//		errors.push_back(std::move(handleHashMapResult.GetError()));
//		return errors;
//	}
//
//	const auto& handleHashMap = handleHashMapResult.GetValue();
//
//	// deserialize entities
//	auto checkEnts = util::CheckJsonKey(j, kJsonEntitiesKey, 
//										nlohmann::json::value_t::array);
//	if (!checkEnts.Success())
//	{
//		errors.push_back(std::move(checkEnts.GetError()));
//		return errors;
//	}
//
//	size_t entityCount = 0;
//	for (const auto& entityJson : j.at(kJsonEntitiesKey))
//	{
//		if (!entityJson.contains(kJsonComponentsKey))
//		{
//			errors.push_back(MAKE_ERROR_FMT(
//				"Components JSON key not found in entity field '{}',", entityCount));
//
//			++entityCount;
//			continue;
//		}
//
//		const auto& componentsJson = entityJson.at(kJsonComponentsKey);
//
//		auto deserializationTarget = MakeComponentDeserializationTarget();
//
//		from_json(componentsJson, deserializationTarget);
//
//		auto entity = ECS::CreateEntity();
//		assert(entity.IsValid());
//
//		auto tryAddComponent = [&](auto&& fieldResult) {
//			if (!fieldResult.Success())
//			{
//				errors.push_back(std::move(fieldResult.GetError()));
//				return;
//			}
//
//			auto& fieldOptional = fieldResult.GetValue();
//			if (!fieldOptional.has_value())
//			{
//				return;
//			}
//
//			entity.AddComponent(std::move(*fieldOptional));
//		};
//
//		ForEachInTuple(std::move(deserializationTarget), tryAddComponent);
//
//		auto resolveResult = util::ComponentSerializationResolver::Deserialize(
//			entity, textureRepo, componentsJson, handleHashMap
//		);
//		if (!resolveResult.Success())
//		{
//			errors.push_back(std::move(resolveResult.GetError()));
//		}
//
//		++entityCount;
//	}
//
//	return errors;
//}
