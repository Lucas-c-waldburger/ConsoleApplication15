#pragma once
#include "user_types/ComponentJsonUserType.h"
#include "../systems/SDLInputSystem.h"
#include "SerializationConcepts.h"

struct DeserializerContext
{
	explicit DeserializerContext(std::vector<Error>& errs) : errors(errs) {}

	std::vector<Error>& errors;
	Entity entity;
};

template <typename T>
concept SomeDeserializerContext =
std::derived_from<T, DeserializerContext> && HasFromJson<T>;

struct PhysicsDeserializerContext : DeserializerContext
{
	struct ChildColliderData
	{
		Entity childEntity;
		size_t parentBodyId = std::numeric_limits<size_t>::max();
		ComponentBuilder<Collider> builder;
	};

	PhysicsDeserializerContext(std::vector<Error>& errs, B2World& w) 
		: DeserializerContext(errs), world(w) {}

	B2World& world;
	std::unordered_map<size_t, Entity> bodyIdToEntityMap;
	std::vector<ChildColliderData> childCollidersToResolve;
};

struct RelationDeserializerContext : DeserializerContext
{
	RelationDeserializerContext(std::vector<Error>& errs, EntityPassKey key)
		: DeserializerContext(errs), passKey(key) {}

	EntityPassKey passKey;
	std::unordered_map<Entity_t, Entity> oldIdsToParentEntities;
	std::vector<Entity> childrenToResolve;
};

struct SpriteDeserializerContext : DeserializerContext
{
	SpriteDeserializerContext(std::vector<Error>& errs, const SpriteAtlas& atlas)
		: DeserializerContext(errs), spriteAtlas(atlas) {}

	const SpriteAtlas& spriteAtlas;
};

struct TextDeserializerContext : DeserializerContext
{
	TextDeserializerContext(std::vector<Error>& errs, const FontAtlas& atlas)
		: DeserializerContext(errs), fontAtlas(atlas) {}

	const FontAtlas& fontAtlas;
};

struct GameControllerDeserializerContext : DeserializerContext
{
	GameControllerDeserializerContext(std::vector<Error>& errs, 
		const GameControllerEventHandler& gcHandler) : 
		DeserializerContext(errs), gameControllerHandler(gcHandler) {}

	const GameControllerEventHandler& gameControllerHandler;
};

class EntityDeserializer : EntityFullAccessPrivelage
{
public:
	EntityDeserializer(B2World& world, const TextureRepository& repo, const SDLInputSystem& inpSys) : 
		relationContext_(deserializationErrors_, GetEntityPassKey()),
		physicsContext_(deserializationErrors_, world),
		spriteContext_(deserializationErrors_, repo.GetSpriteAtlas()),
		textContext_(deserializationErrors_, repo.GetFontAtlas()),
		gameControllerContext_(deserializationErrors_, inpSys.GetGameControllerEventHandler()) {}

	std::vector<Error> DeserializeEntities(const std::string& jsonFilepath);

private:
	void DeserializeBasicComponents(const nlohmann::json& entityJ, Entity& e);
	void DeserializeContextComponents(const nlohmann::json& entityJ);
	void DeserializeUserComponents(const nlohmann::json& entityJ, Entity& e);

	void UpdateContexts(Entity& e);

	void ResolvePhysics();
	void ResolveRelations();

	RelationDeserializerContext relationContext_;
	PhysicsDeserializerContext physicsContext_;
	SpriteDeserializerContext spriteContext_;
	TextDeserializerContext textContext_;
	GameControllerDeserializerContext gameControllerContext_;
	std::vector<Error> deserializationErrors_;
};