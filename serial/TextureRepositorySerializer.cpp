#include "TextureRepositorySerializer.h"
#include "SerializationUtils.h"

Result<Void> 
TextureRepositorySerializer::Serialize(const std::string& jsonFilepath, const TextureRepository& repo)
{
	std::ofstream file(jsonFilepath);
	if (!file)
	{
		return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'", jsonFilepath);
	}

	nlohmann::json j;

	auto& texturesJ = j["textures"] = nlohmann::json::object();

	auto& spritesJ = texturesJ["spriteAtlas"] = nlohmann::json::array();
	auto spritePackage = repo.GetSpriteAtlas().ExportSpriteDescriptors();
	for (const auto& descriptors : spritePackage)
	{
		spritesJ.push_back(descriptors);
	}

	auto& fontsJ = texturesJ["fontAtlas"] = nlohmann::json::array(); 
	auto fontPackage = repo.GetFontAtlas().ExportFontDescriptors();
	for (const auto& descriptors : fontPackage)
	{
		fontsJ.push_back(descriptors);
	}

	file << j.dump(4);

	return kVoid;
}

std::vector<Error>
TextureRepositorySerializer::Deserialize(const std::string& jsonFilepath, TextureRepository& repo,
										 SDL_Renderer* renderer)
{
	if (!renderer)
	{
		return { MAKE_ERROR("Renderer was null") };
	}

	auto jResult = LoadJson(jsonFilepath);
	if (!jResult.Success())
	{
		return { jResult.GetError() };
	}
	const auto& j = jResult.GetValue();

	if (!j.contains("textures"))
	{
		return { MAKE_ERROR("JSON file does not contain 'textures' field") };
	}
	const auto& texturesJ = j.at("textures");

	std::vector<Error> errors{};

	if (texturesJ.contains("spriteAtlas"))
	{
		const auto& spriteAtlasJ = texturesJ.at("spriteAtlas");
		if (!spriteAtlasJ.is_array())
		{
			errors.emplace_back(MAKE_ERROR("'spriteAtlas' field in JSON file was not of type array"));
		}
		else
		{
			SpriteDescriptorPackage spritePackage;
			spritePackage.reserve(spriteAtlasJ.size());

			try
			{
				from_json(spriteAtlasJ, spritePackage);
			}
			catch (const nlohmann::json::exception& err)
			{
				errors.emplace_back(
					MAKE_ERROR_FMT("Error parsing sprite atlas descriptors: '{}'", err.what())
				);
			}

			for (auto&& descriptors : spritePackage)
			{
				auto loadResult = repo.GetSpriteAtlas().LoadSprites(renderer, std::move(descriptors));
				if (!loadResult.Success())
				{
					errors.emplace_back(std::move(loadResult.GetError()));
				}
			}
		}
	}
	else
	{
		errors.emplace_back(MAKE_ERROR("JSON file does not contain 'spriteAtlas' field"));
	}

	if (texturesJ.contains("fontAtlas"))
	{
		const auto& fontAtlasJ = texturesJ.at("fontAtlas");
		if (!fontAtlasJ.is_array())
		{
			errors.emplace_back(MAKE_ERROR("'fontAtlas' field in JSON file was not of type array"));
		}
		else
		{
			FontDescriptors fontDescriptors;
			fontDescriptors.reserve(fontAtlasJ.size());

			try
			{
				from_json(fontAtlasJ, fontDescriptors);
			}
			catch (const nlohmann::json::exception& err)
			{
				errors.emplace_back(
					MAKE_ERROR_FMT("Error parsing font atlas descriptors: '{}'", err.what())
				);
			}

			auto loadResult = repo.GetFontAtlas().LoadFonts(renderer, std::move(fontDescriptors));
			if (!loadResult.Success())
			{
				errors.emplace_back(std::move(loadResult.GetError()));
			}
		}
	}
	else
	{
		errors.emplace_back(MAKE_ERROR("JSON file does not contain 'fontAtlas' field"));
	}

	return errors;
}