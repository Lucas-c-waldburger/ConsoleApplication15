#pragma once
#include "../../atlas/NewTextureRepository.h"
#include "../../serial/SerializationUtils.h"

class Entity;

//template <typename T> struct ComponentSerializer;
//template <typename T> struct ComponentDeserializer;
//
//template <typename T>
//concept ExtraSerializationSteps = requires() {
//
//}
//
//struct ComponentSerializationResolver
//{
//	template <typename T> requires 
//};


inline constexpr std::string_view kTextureAtlasesKey = "textureAtlases";
inline constexpr std::string_view kGlyphAtlasesKey = "glyphAtlases";
inline constexpr std::string_view kSpriteAtlasesKey = "spriteAtlases";
inline constexpr std::string_view kSpriteKey = "sprite";
inline constexpr std::string_view kSourceAtlasHashKey = "sourceAtlasHash";
inline constexpr std::string_view kWriterKey = "writer";
inline constexpr std::string_view kTextKey = "text";

namespace util {

Result<Void> CheckJsonKey(const nlohmann::json& j, std::string_view key,
						  std::optional<nlohmann::json::value_t> expectedValueType = {});

using AtlasHandleHashMap = std::unordered_map<size_t, Handle<TextureAtlas>>;

class TextureRepositorySerializationHelper
{
public:
	static Result<Void> SerializeAtlases(const TextureRepository& repo,
										 nlohmann::json& j);

	static Result<AtlasHandleHashMap> DeserializeAtlases(SDL_Renderer* renderer, 
														 TextureRepository& repo,
														 const nlohmann::json& j);

private:
	static Result<Void> DeserializeSpriteAtlases(SDL_Renderer* renderer, 
												 TextureRepository& repo, 
												 const nlohmann::json& j,
												 AtlasHandleHashMap& handleHashMap);
	static Result<Void> DeserializeGlyphAtlases(SDL_Renderer* renderer,
												TextureRepository& repo,
												const nlohmann::json& j,
												AtlasHandleHashMap& handleHashMap);

	TextureRepositorySerializationHelper() = default;
};

class RenderableSerializationHelper
{ 
public:
	static Result<Void> SerializeSprite(const Sprite& sprite, 
										const TextureRepository& repo,
										nlohmann::json& spriteComponentJson);
	static Result<Sprite>
	DeserializeSprite(const nlohmann::json& spriteComponentJson,
					  const TextureRepository& repo,
					  const AtlasHandleHashMap& handleHashMap);

	static Result<Void> SerializeGlyphTextWriter(const GlyphTextWriter& writer,
												 const TextureRepository& repo,
												 nlohmann::json& textComponentJson);
	static Result<GlyphTextWriter>
	DeserializeGlyphTextWriter(const nlohmann::json& textComponentJson,
							   const TextureRepository& repo,
							   const AtlasHandleHashMap& handleHashMap);

private:
	RenderableSerializationHelper() = default;
};


class ComponentSerializationResolver
{
public:
	static Result<Void> Serialize(const Entity& entity, const TextureRepository& repo, 
								  nlohmann::json& componentsJson);
	static Result<Void> Deserialize(Entity& entity, const TextureRepository& repo, 
									const nlohmann::json& componentsJson,
									const AtlasHandleHashMap& handleHashMap);

private:
	ComponentSerializationResolver() = default;
};




} // util