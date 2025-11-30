#pragma once
#include "../../atlas/NewTextureRepository.h"
#include "../../deps/nlohmann/json.hpp"

class Entity;



namespace util {

//using EntityToSpriteInfoMap = std::unordered_map<Entity, SpriteInfo>;
//
//
//class SpriteDescriptorPackageMaker
//{
//public:
//	SpriteDescriptorPackageMaker();
//
//	Result<Void> PushBack(SpriteInfo&& info);
//
//	auto begin() { return packages_.begin(); }
//	auto end() { return packages_.end(); }
//
//private:
//	static constexpr std::string_view kNoSeriesName = "__no_series";
//
//	std::unordered_map<std::string_view, size_t> spriteSeriesNameToPackageIndex_;
//	std::vector<SpriteDescriptorPackage> packages_;
//};

class TextureRepositorySerializationHelper
{
public:
	TextureRepositorySerializationHelper(TextureRepository& repo, SDL_Renderer* renderer) :
		textureRepository_(repo), renderer_(renderer) 
	{}

	//Result<Void> SerializeAtlases

	static Result<Void> SerializeSpriteAtlas(const DynamicSpriteAtlas& atlas);
	static Result<FixedSpriteAtlas> DeserializeSpriteAtlas(nlohmann::json& j, SDL_Renderer* renderer);

private:
	TextureRepository& textureRepository_;
	SDL_Renderer* renderer_ = nullptr;
};

class RenderableSerializationHelper
{
public:
	static constexpr std::string_view kFilepathJsonKey = "filepath";
	static constexpr std::string_view kSpriteInfoJsonKey = "spriteInfo";
	static constexpr std::string_view kSourceAtlasJsonKey = "sourceAtlas";
	static constexpr std::string_view kHandleHashJsonKey = "hash";

	RenderableSerializationHelper(TextureRepository& repo, SDL_Renderer* renderer) :
		textureRepository_(repo), renderer_(renderer) {}

	Result<Void> SerializeSprite(const Sprite& sprite, nlohmann::json& spriteComponentJson);
	Result<Sprite> DeserializeSprite(const nlohmann::json& spriteComponentJson);

private:
	TextureRepository& textureRepository_;
	SDL_Renderer* renderer_ = nullptr;
	std::unordered_map<size_t, Handle<TextureAtlas>> atlasHandleHashMap_;
};


class ComponentSerializationResolver
{
public:
	explicit ComponentSerializationResolver(TextureRepository& repo, SDL_Renderer* renderer) :
		renderableSerializationHelper_(repo, renderer) {}

	Result<Void> Serialize(const Entity& entity, nlohmann::json& componentsJson);
	Result<Void> Deserialize(Entity& entity, const nlohmann::json& componentsJson);

private:
	RenderableSerializationHelper renderableSerializationHelper_;
};



} // util