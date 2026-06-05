#pragma once
#include "user_types/ComponentJsonUserType.h"
#include "../atlas/NewTextureRepository.h"
#include "../audio/AudioBank.h"

struct SpriteSerializerContext 
{
	Entity entity;
	const SpriteAtlas& spriteAtlas;
};

struct TextSerializerContext
{
	Entity entity;
	const FontAtlas& fontAtlas;
};

struct AudioSerializerContext
{
	Entity entity;
	const AudioBank& audioBank;
};

class EntitySerializer : EntityFullAccessPrivelage
{
public:
	EntitySerializer(const TextureRepository& repo, const AudioBank& bank) : 
		spriteContext_({ .spriteAtlas = repo.GetSpriteAtlas() }),
		textContext_({ .fontAtlas = repo.GetFontAtlas() }),
		audioContext_({ .audioBank = bank })
	{}

	Result<Void> SerializeEntities(const std::string& jsonFilepath);

private:
	static void SerializeBasicComponents(nlohmann::json& entityJ, const Entity& e);
	void SerializeContextComponents(nlohmann::json& entityJ);

	void UpdateContexts(const Entity& e);

	SpriteSerializerContext spriteContext_;
	TextSerializerContext textContext_;
	AudioSerializerContext audioContext_;
	Error serializationError_;
};