#include "SerializationSystem.h"
#include <filesystem>
#include "../serial/user_types/EntityJsonUserType.h"


Result<Void> SerializationSystem::SerializeEntities(const ResourcePathResult& path)
{
	if (!path.Success())
	{
		return path.GetError();
	}

	auto entities = ECS::GetAllActiveEntities();
	if (entities.empty())
	{
		return;
	}

	std::ofstream file(path.GetValue());
	if (!file) 
	{
		return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'", path.GetValue());
	}

	nlohmann::json j;

	for (const auto& entity : entities)
	{
		j[kJsonEntitiesKey].push_back(entity);
	}

	file << std::setw(4) << j;

	return Void{};
}

std::vector<Error> SerializationSystem::DeserializeEntities(const ResourcePathResult& path)
{
	std::vector<Error> errors{};

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

	size_t entityFieldCount = 0;
	for (const auto& entityField : j.at(kJsonEntitiesKey))
	{
		if (!entityField.contains(kJsonComponentsKey))
		{
			errors.push_back(MAKE_ERROR_FMT(
				"Components JSON key not found in entity field '{}',", entityFieldCount));
			continue;
		}

		auto deserializationTarget = MakeComponentDeserializationTarget();

		from_json(entityField.at(kJsonComponentsKey), deserializationTarget);

		auto entity = ECS::CreateEntity();
		assert(entity.IsValid());

		auto tryAddComponent = [&](auto&& fieldResult) {
			if (!fieldResult.Success())
			{
				errors.push_back(std::move(field.GetError()));
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

		++entityFieldCount;
	}

	return errors;
}
