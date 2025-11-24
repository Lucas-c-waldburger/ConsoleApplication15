#include "SerializationSystem.h"
#include <filesystem>
#include "../serial/user_types/EntityJsonUserType.h"
#include "util/SerializationSystemUtils.h"

namespace {

//Result<Void> SerializeSprite(const Sprite& sprite, nlohmann::json& j, 
//							 TextureRepository& textureRepo)
//{
//	static constexpr std::string_view spriteComponentName = 
//		ComponentName<SpriteRenderableComponent>::value;
//	if (!j.contains(spriteComponentName))
//	{
//		return MAKE_ERROR_FMT("Entity's components JSON field was not "
//			"populated with key '{}'", spriteComponentName);
//	}
//
//	if (!sprite.sourceAtlas.IsValid())
//	{
//		LOG_ERROR("Sprite source atlas handle was marked invalid");
//
//		return Void{};
//	}
//
//	auto* spriteAtlas = textureRepo.GetAtlas<SpriteAtlas>(sprite.sourceAtlas);
//	if (!spriteAtlas)
//	{
//		return MAKE_ERROR("Sprite source atlas not found in Texture Repository");
//	}
//
//	const auto& info = spriteAtlas->GetSpriteInfo(sprite);
//	if (info.filepath.empty())
//	{
//		return MAKE_ERROR("Sprite filepath was empty, meaning sprite index was invalid");
//	}
//
//	if (!std::filesystem::exists(info.filepath))
//	{
//		return MAKE_ERROR("Sprite filepath does not exist");
//	}
//
//	auto& spriteComponentField = j.at(spriteComponentName);
//	spriteComponentField["filepath"] = info.filepath;
//
//	return Void{};
//}


//Result<Void> DeserializeSprite(Sprite& sprite, std::string_view spriteFilepath,
//							   TextureRepository& textureRepo)
//{
//
//}

}

Result<Void> SerializationSystem::SerializeEntities(std::string_view jsonFilename, 
													TextureRepository& textureRepo)
{
	auto entities = ECS::GetAllActiveEntities();
	if (entities.empty())
	{
		return Void{};
	}

	auto path = JoinPathsRaw(kJsonDirName, jsonFilename);

	std::ofstream file(path);
	if (!file) 
	{
		return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'", path.string());
	}

	nlohmann::json j;
	auto& entityArr = j[kJsonEntitiesKey] = nlohmann::json::array();

	util::ComponentSerializationResolver serializationResolver{ 
		textureRepo, SDLite::Renderer() };

	for (const auto& entity : entities)
	{
		auto& entityField = entityArr.emplace_back(entity);
		if (!entityField.contains(kJsonComponentsKey))
		{
			return MAKE_ERROR("Entity was not serialized with a components JSON field");
		}

		auto& componentsField = entityField.at(kJsonComponentsKey);

		auto resolveResult = serializationResolver.Serialize(entity, componentsField);
		if (!resolveResult.Success())
		{
			LOG_ERROR(resolveResult.GetError());
		}
	}

	file << std::setw(4) << j;

	return Void{};
}

std::vector<Error> SerializationSystem::DeserializeEntities(
	std::string_view jsonFilename, TextureRepository& textureRepo)
{
	std::vector<Error> errors{};

	auto path = ResourcePath::Json(jsonFilename);
	if (!path.Success())
	{
		errors.push_back(path.GetError());
		return errors;
	}

	std::ifstream file(path.GetValue());
	if (!file)
	{
		errors.push_back(MAKE_ERROR_FMT("Could not open exisiting JSON "
			" file at path: '{}'", path.GetValue()));
		return errors;
	}

	nlohmann::json j;
	try 
	{
		j = nlohmann::json::parse(file);
	}
	catch (const nlohmann::json::parse_error& err) 
	{
		errors.push_back(MAKE_ERROR_FMT("JSON parse error: '{}'", err.what()));
		return errors;
	}

	if (!j.contains(kJsonEntitiesKey))
	{
		errors.push_back(MAKE_ERROR("Entities JSON key not found"));
		return errors;
	}

	util::ComponentSerializationResolver serializationResolver{
		textureRepo, SDLite::Renderer() };

	size_t entityFieldCount = 0;
	for (const auto& entityField : j.at(kJsonEntitiesKey))
	{
		if (!entityField.contains(kJsonComponentsKey))
		{
			errors.push_back(MAKE_ERROR_FMT(
				"Components JSON key not found in entity field '{}',", entityFieldCount));

			++entityFieldCount;
			continue;
		}

		auto& componentsField = entityField.at(kJsonComponentsKey);

		auto deserializationTarget = MakeComponentDeserializationTarget();

		from_json(entityField.at(componentsField), deserializationTarget);

		auto entity = ECS::CreateEntity();
		assert(entity.IsValid());

		auto tryAddComponent = [&](auto&& fieldResult) {
			if (!fieldResult.Success())
			{
				errors.push_back(std::move(fieldResult.GetError()));
				return;
			}

			auto& fieldOptional = fieldResult.GetValue();
			if (!fieldOptional.has_value())
			{
				return;
			}

			entity.AddComponent(std::move(*fieldOptional));
		};

		ForEachInTuple(std::move(deserializationTarget), tryAddComponent);

		auto resolveResult = serializationResolver.Deserialize(entity, componentsField);
		if (!resolveResult.Success())
		{
			errors.push_back(std::move(resolveResult.GetError()));
		}

		++entityFieldCount;
	}

	return errors;
}
