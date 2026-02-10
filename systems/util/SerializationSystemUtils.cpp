#include "SerializationSystemUtils.h"
#include "../../core/Algorithms.h"
#include "../../serial/user_types/AtlasJsonUserTypes.h"
#include "../../serial/user_types/RenderableJsonUserTypes.h"
#include "../../serial/Serialization.h"
#include "../../components/RenderableComponent.h"
#include "../../ecs/Ecs.h"

namespace util {

Result<Void> CheckJsonKey(const nlohmann::json& j, std::string_view key,
						  std::optional<nlohmann::json::value_t> expectedValueType)
{
	if (!j.contains(key))
	{
		return MAKE_ERROR_FMT("Json missing key: '{}'", key);
	}
	if (expectedValueType.has_value() && *expectedValueType != j.at(key).type())
	{
		return MAKE_ERROR_FMT("Json type '{}' differs from expected type", j.type_name());
	}

	return Void{};
}

Result<Void> 
TextureRepositorySerializationHelper::SerializeAtlases(const TextureRepository& repo,
													   nlohmann::json& j)
{
	//j[kSpriteAtlasesKey] = repo.GetAtlasVector<SpriteAtlasTexture>();
	//j[kGlyphAtlasesKey] = repo.GetAtlasVector<GlyphAtlas>();

	return Void{};
}



Result<AtlasHandleHashMap> 
TextureRepositorySerializationHelper::DeserializeAtlases(SDL_Renderer* renderer, 
														 TextureRepository& repo, 
														 const nlohmann::json& j)										
{
	assert(renderer);
	AtlasHandleHashMap handleHashMap{};

	TRY(CheckJsonKey(j, kSpriteAtlasesKey, nlohmann::json::value_t::array));
	TRY(DeserializeSpriteAtlases(renderer, repo, j.at(kSpriteAtlasesKey), handleHashMap));

	TRY(CheckJsonKey(j, kGlyphAtlasesKey, nlohmann::json::value_t::array));
	TRY(DeserializeGlyphAtlases(renderer, repo, j.at(kGlyphAtlasesKey), handleHashMap));

	return handleHashMap;
}

Result<Void> 
TextureRepositorySerializationHelper::DeserializeSpriteAtlases(SDL_Renderer* renderer,
															   TextureRepository& repo,
															   const nlohmann::json& j,
															   AtlasHandleHashMap& handleHashMap)
{
	handleHashMap.reserve(handleHashMap.size() + j.size());
	SpriteDescriptorPackage package;

	for (const auto& atlasJson : j)
	{
		TRY(CheckJsonKey(atlasJson, kSpriteDescriptorsKey, nlohmann::json::value_t::array));

		TRY(GetJsonNativeValue<size_t>(atlasJson, kTextureSizeKey), textureSize);
		TRY(GetJsonNativeValue<size_t>(atlasJson, kHashKey), hash);

		if (handleHashMap.contains(hash))
		{
			return MAKE_ERROR("Duplicate hash value for sprite atlas");
		}

		//TRY(repo.CreateAtlas<SpriteAtlasTexture>(renderer, textureSize), atlas);
		//assert(atlas);

		auto& spriteAtlas = repo.GetSpriteAtlas();
		//handleHashMap[hash] = atlas->GetHandle();

		package.clear();
		atlasJson.at(kSpriteDescriptorsKey).get_to(package);
		
		for (auto&& descriptors : package)
		{
			TRY(spriteAtlas.LoadSprites(renderer, std::move(descriptors)));
		}
	}

	return Void{};
}

Result<Void>
TextureRepositorySerializationHelper::DeserializeGlyphAtlases(SDL_Renderer* renderer, 
															  TextureRepository& repo, 
															  const nlohmann::json& j, 
															  AtlasHandleHashMap& handleHashMap)
{
	handleHashMap.reserve(handleHashMap.size() + j.size());
	
	for (const auto& atlasJson : j)
	{
		TRY(CheckJsonKey(atlasJson, kFontDescriptorKey));

		TRY(GetJsonNativeValue<size_t>(atlasJson, kHashKey), hash);

		if (handleHashMap.contains(hash))
		{
			return MAKE_ERROR("Duplicate hash value for glyph atlas");
		}

		//TRY(repo.CreateAtlas<GlyphAtlas>(renderer,
		//	atlasJson.at(kFontDescriptorKey).get<FontDescriptor>()), atlas);
		//assert(atlas);

		//handleHashMap[hash] = atlas->GetHandle();
	}

	return Void{};
}

Result<Void> RenderableSerializationHelper::SerializeSprite(const Sprite& sprite, 
	const TextureRepository& repo, nlohmann::json& spriteComponentJson)
{
	if (!sprite.resourceHandle.IsValid())
	{
		LOG_ERROR("Sprite source atlas handle was marked invalid");

		return Void{};
	}

	//const auto* spriteAtlas = repo.GetAtlas<SpriteAtlasTexture>(sprite.resourceHandle);
	//if (!spriteAtlas)
	//{
	//	return MAKE_ERROR("Sprite source atlas not found in Texture Repository");
	//}

	const auto& spriteAtlas = repo.GetSpriteAtlas();

	auto spriteName = spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName>(sprite);
	if (!spriteName.has_value())
	{
		return MAKE_ERROR("Sprite not found in sprite atlas");
	}

	spriteComponentJson[kSpriteKey] = {
		{ kSourceAtlasHashKey, sprite.resourceHandle.GetHash() },
		{ kSpriteNameKey, *spriteName }
	};

	return Void{};
}

Result<Sprite> RenderableSerializationHelper::DeserializeSprite(
	const nlohmann::json& spriteComponentJson, const TextureRepository& repo, 
	const AtlasHandleHashMap& handleHashMap)
{
	TRY(CheckJsonKey(spriteComponentJson, kSpriteKey));
	const auto& spriteJson = spriteComponentJson.at(kSpriteKey);

	TRY(GetJsonNativeValue<std::string>(spriteJson, kSpriteNameKey), spriteName);
	TRY(GetJsonNativeValue<size_t>(spriteJson, kSourceAtlasHashKey), sourceAtlasHash);

	auto it = handleHashMap.find(sourceAtlasHash);
	if (it == handleHashMap.end())
	{
		return MAKE_ERROR("Sprite serialized with unrecognized source atlas hash");
	}

	//const auto* spriteAtlas = repo.GetAtlas<SpriteAtlasTexture>(it->second);
	//if (!spriteAtlas)
	//{
	//	return MAKE_ERROR("Handle not found in texture repository");
	//}
	const auto& spriteAtlas = repo.GetSpriteAtlas();

	auto sprite = spriteAtlas.GetSprite(spriteName);
	if (!sprite.resourceHandle.IsValid())
	{
		return MAKE_ERROR("Sprite not found in sprite atlas");
	}

	return sprite;
}

Result<Void> RenderableSerializationHelper::SerializeGlyphTextWriter(
	const GlyphTextWriter& writer, const TextureRepository& repo, 
	nlohmann::json& textComponentJson)
{
	if (!writer.resourceHandle.IsValid())
	{
		LOG_ERROR("Glyph text writer source atlas handle was marked invalid");

		return Void{};
	}

	//if (!repo.HasAtlas(writer.resourceHandle))
	if (!repo.GetFontAtlas().HasFont(writer.resourceHandle))
	{
		return MAKE_ERROR("Glyph text writer source atlas not found in Texture Repository");
	}

	textComponentJson[kWriterKey] = {
		{ kSourceAtlasHashKey, writer.resourceHandle.GetHash() },
		{ kTextKey, writer.text }
	};

	return Void{};
}

Result<GlyphTextWriter> RenderableSerializationHelper::DeserializeGlyphTextWriter(
	const nlohmann::json& textComponentJson, const TextureRepository& repo, 
	const AtlasHandleHashMap& handleHashMap)
{
	TRY(CheckJsonKey(textComponentJson, kSpriteKey));
	const auto& writerJson = textComponentJson.at(kWriterKey);

	TRY(GetJsonNativeValue<size_t>(writerJson, kSourceAtlasHashKey), sourceAtlasHash);
	TRY(GetJsonNativeValue<std::string>(writerJson, kTextKey), writerText);

	auto it = handleHashMap.find(sourceAtlasHash);
	if (it == handleHashMap.end())
	{
		return MAKE_ERROR("Text writer serialized with unrecognized source atlas hash");
	}

	//const auto* glyphAtlas = repo.GetAtlas<GlyphAtlas>(it->second);
	//if (!glyphAtlas)
	//{
	//	return MAKE_ERROR("Handle not found in texture repository");
	//}
	//const auto& fontAtlas = repo.GetFontAtlas();
	return GlyphTextWriter{};

	//auto writer = fontAtlas.GetTextWriter();
	//writer.text = std::move(writerText);

	//return writer;
}

Result<Void> ComponentSerializationResolver::Serialize(
	const Entity& entity, const TextureRepository& repo, 
	nlohmann::json& componentsJson)
{
	if (!entity.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}
	if (!componentsJson.is_object())
	{
		return MAKE_ERROR("Components JSON was not of type object");
	}

	if (entity.HasComponent<SpriteRenderableComponent>())
	{
		static constexpr std::string_view spriteComponentName =
			ComponentName<SpriteRenderableComponent>::value;

		TRY(CheckJsonKey(componentsJson, spriteComponentName, 
			nlohmann::json::value_t::object));

		TRY(RenderableSerializationHelper::SerializeSprite(
			entity.GetComponent<SpriteRenderableComponent>().sprite,
			repo, componentsJson.at(spriteComponentName)));
	}
	else if (entity.HasComponent<TextRenderableComponent>())
	{
		static constexpr std::string_view textComponentName =
			ComponentName<TextRenderableComponent>::value;

		TRY(CheckJsonKey(componentsJson, textComponentName,
			nlohmann::json::value_t::object));

		TRY(RenderableSerializationHelper::SerializeGlyphTextWriter(
			entity.GetComponent<TextRenderableComponent>().writer,
			repo, componentsJson.at(textComponentName)));
	}

	return Void{};
}

Result<Void> ComponentSerializationResolver::Deserialize(Entity& entity,
	const TextureRepository& repo, const nlohmann::json& componentsJson, 
	const AtlasHandleHashMap& handleHashMap)
{
	if (!entity.IsValid())
	{
		return MAKE_ERROR("Entity was invalid");
	}
	if (!componentsJson.is_object())
	{
		return MAKE_ERROR("Components JSON was not of type object");
	}

	if (entity.HasComponent<SpriteRenderableComponent>())
	{
		static constexpr std::string_view spriteComponentName =
			ComponentName<SpriteRenderableComponent>::value;

		TRY(CheckJsonKey(componentsJson, spriteComponentName,
			nlohmann::json::value_t::object));

		TRY_ASSIGN(entity.GetComponent<SpriteRenderableComponent>().sprite,
			RenderableSerializationHelper::DeserializeSprite(
			componentsJson.at(spriteComponentName), repo, handleHashMap));
	}
	else if (entity.HasComponent<TextRenderableComponent>())
	{
		static constexpr std::string_view textComponentName =
			ComponentName<TextRenderableComponent>::value;

		TRY(CheckJsonKey(componentsJson, textComponentName,
			nlohmann::json::value_t::object));

		TRY_ASSIGN(entity.GetComponent<TextRenderableComponent>().writer,
			RenderableSerializationHelper::DeserializeGlyphTextWriter(
			componentsJson.at(textComponentName), repo, handleHashMap));
	}

	return Void{};
}

} //util
