#include "EntityDeserializer.h"
#include "SerializationUtils.h"
#include "../atlas/NewTextureRepository.h"

template <typename BasicJson>
inline void from_json(const BasicJson& j, PhysicsDeserializerContext& ctx)
{
	static constexpr auto addCollider = [](Entity& rbEnt, ComponentBuilder<Collider>& builder) {
		assert(rbEnt.IsValid());
		assert(rbEnt.HasComponent<RigidBody>());
		auto& rb = rbEnt.GetComponent<RigidBody>();
		auto& body = WriteAccessor<B2Body>{}(rb.body);
		assert(body.IsValid());

		rbEnt.AddComponent(builder.Build(body));
	};

	assert(ctx.entity.IsValid());
	assert(ctx.world.IsValid());

	size_t bodyId = std::numeric_limits<size_t>::max();

	if (j.contains("RigidBody"))
	{
		const auto& rbJ = j.at("RigidBody");
		if (!rbJ.contains("bodyId"))
		{
			ctx.errors.emplace_back(MAKE_ERROR("RigidBody component JSON must contain bodyId field"));
			return;
		}

		try
		{
			from_json(rbJ.at("bodyId"), bodyId);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing bodyId field for RigidBody component: {}", err.what())
			);
			return;
		}

		auto [_, inserted] = ctx.bodyIdToEntityMap.emplace(bodyId, ctx.entity);
		assert(inserted);

		ComponentBuilder<RigidBody> builder{};

		if (rbJ.contains("bodyLimits"))
		{
			try 			
			{
				builder.WithBodyLimits(rbJ.at("bodyLimits").get<BodyLimits>());
			}
			catch (const nlohmann::json::exception& err)
			{
				ctx.errors.emplace_back(
					MAKE_ERROR_FMT("Error parsing bodyLimits field for RigidBody component: {}", err.what())
				);
				return;
			}
		}

		if (!rbJ.contains("bodyParameters"))
		{
			ctx.errors.emplace_back(
				MAKE_ERROR("RigidBody component JSON must contain bodyParameters field")
			);
			return;
		}
		try
		{
			builder.WithBodyParameters(rbJ.at("bodyParameters").get<BodyParameters>());
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing bodyParameters field for RigidBody component: {}", err.what())
			);
			return;
		}

		ctx.entity.AddComponent(builder.Build(ctx.world));
	}

	if (j.contains("Collider"))
	{
		const auto& colliderJ = j.at("Collider");
		if (!colliderJ.contains("parentBodyId"))
		{
			ctx.errors.emplace_back(MAKE_ERROR("Collider component JSON must contain parentBodyId field"));
			return;
		}

		size_t parentBodyId = std::numeric_limits<size_t>::max();
		try
		{
			from_json(colliderJ.at("parentBodyId"), parentBodyId);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing parentBodyId field for Collider component: {}", err.what())
			);
			return;
		}

		ComponentBuilder<Collider> builder{};

		if (colliderJ.contains("colliderSettings"))
		{
			try
			{
				builder.WithColliderSettings(colliderJ.at("colliderSettings").get<ColliderSettings>());
			}
			catch (const nlohmann::json::exception& err)
			{
				ctx.errors.emplace_back(
					MAKE_ERROR_FMT("Error parsing colliderSettings field for Collider component: {}", err.what())
				);
				return;
			}
		}

		if (!colliderJ.contains("shapeParameters"))
		{
			ctx.errors.emplace_back(
				MAKE_ERROR("Collider component JSON must contain shapeParameters field")
			);
			return;
		}
		try
		{
			builder.WithShapeParameters(colliderJ.at("shapeParameters").get<B2ShapeParameters>());
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing shapeParameters field for Collider component: {}", err.what())
			);
			return;
		}

		if (bodyId == parentBodyId)
		{
			addCollider(ctx.entity, builder);
		}
		else
		{
			ctx.childCollidersToResolve.emplace_back(
				ctx.entity, parentBodyId, std::move(builder)
			);
		}
	}
}

template <typename BasicJson>
void from_json(const BasicJson& j, RelationDeserializerContext& ctx)
{
	if (j.contains("Parent"))
	{
		assert(!j.contains("Children"));

		Parent parentCmp{};
		try
		{
			from_json(j.at("Parent"), parentCmp);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing parent field for Relation component: {}", err.what())
			);
			return;
		}

		ctx.entity.AddComponent(std::move(parentCmp), ctx.passKey);
		ctx.childrenToResolve.emplace_back(ctx.entity);
	}
	else if (j.contains("Children"))
	{
		const auto& childrenJ = j.at("Children");
		if (!childrenJ.contains("childEntityIds"))
		{
			ctx.errors.emplace_back(
				MAKE_ERROR("Children component JSON must contain childEntityIds field")
			);
			return;
		}

		const auto& childIdsJ = childrenJ.at("childEntityIds");
		assert(childIdsJ.is_array());
		if (childIdsJ.empty())
		{
			return;
		}

		if (!j.contains("entityId"))
		{
			ctx.errors.emplace_back(MAKE_ERROR("All entities must contain 'entityId' field"));
			return;
		}

		auto [_, inserted] = ctx.oldIdsToParentEntities.emplace(
			j.at("entityId").get<Entity_t>(),
			ctx.entity
		);
		assert(inserted);
	}
}

template <typename BasicJson>
void from_json(const BasicJson& j, SpriteDeserializerContext& ctx)
{
	if (!j.contains("SpriteRenderableComponent"))
	{
		return;
	}
	auto& rendJ = j.at("SpriteRenderableComponent");

	auto& rend = ctx.entity.AddComponent<SpriteRenderableComponent>();

	if (!rendJ.contains("profile"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("SpriteRenderableComponent JSON did not contain 'profile' field. Using default")
		);
	}
	else
	{
		try
		{
			from_json(rendJ.at("profile"), rend.profile);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing SpriteRenderableComponent.profile: {}", err.what())
			);
		}
	}

	if (!rendJ.contains("sprite"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("SpriteRenderableComponent JSON did not contain 'sprite' field")
		);
	}
	else
	{
		std::string spriteName;
		try
		{
			from_json(rendJ.at("sprite"), spriteName);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing SpriteRenderableComponent.sprite: {}", err.what())
			);

			return;
		}

		if (spriteName.empty())
		{
			ctx.errors.emplace_back(
				MAKE_ERROR("SpriteRenderableComponent JSON contained empty 'sprite' field")
			);

			return;
		}

		rend.sprite = ctx.spriteAtlas.GetSprite(spriteName);
		if (!rend.sprite.resourceHandle.IsValid())
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Sprite '{}' not found in SpriteAtlas", spriteName)
			);
		}
	}
}

//// TODO: figure out what to do with 'fontSize'
template <typename BasicJson>
void from_json(const BasicJson& j, TextDeserializerContext& ctx)
{
	if (!j.contains("TextRenderableComponent"))
	{
		return;
	}
	auto& rendJ = j.at("TextRenderableComponent");

	auto& rend = ctx.entity.AddComponent<TextRenderableComponent>();

	if (!rendJ.contains("profile"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("TextRenderableComponent JSON did not contain 'profile' field. Using default")
		);
	}
	else
	{
		try
		{
			from_json(rendJ.at("profile"), rend.profile);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing TextRenderableComponent.profile: {}", err.what())
			);
		}
	}

	if (!rendJ.contains("formatting"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("TextRenderableComponent JSON did not contain 'formatting' field. Using default")
		);
	}
	else
	{
		try
		{
			from_json(rendJ.at("formatting"), rend.formatting);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing TextRenderableComponent.formatting: {}", err.what())
			);
		}
	}

	if (!rendJ.contains("fontName"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("TextRenderableComponent JSON did not contain 'fontName' field")
		);
	}
	else
	{
		std::string fontName;
		try
		{
			from_json(rendJ.at("fontName"), fontName);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing TextRenderableComponent.fontName: {}", err.what())
			);
		}

		if (fontName.empty())
		{
			ctx.errors.emplace_back(
				MAKE_ERROR("TextRenderableComponent JSON contained empty 'fontName' field")
			);
		}
		else
		{
			rend.writer = ctx.fontAtlas.GetTextWriter(fontName);
			if (!rend.writer.resourceHandle.IsValid())
			{
				ctx.errors.emplace_back(
					MAKE_ERROR_FMT("Font '{}' not found in FontAtlas", fontName)
				);
			}
		}
	}

	if (!rendJ.contains("text"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("TextRenderableComponent JSON did not contain 'text' field")
		);
	}
	else
	{
		try
		{
			from_json(rendJ.at("text"), rend.writer.text);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing TextRenderableComponent.writer.text: {}", err.what())
			);

			return;
		}
	}
}

template <typename BasicJson>
void from_json(const BasicJson& j, GameControllerDeserializerContext& ctx)
{
	if (!j.contains("GameControllerState"))
	{
		return;
	}

	auto& gcJ = j.at("GameControllerState");

	auto& gc = ctx.entity.AddComponent<GameControllerState>();

	if (!gcJ.contains("joystickID"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("GameControllerState JSON must contain 'controllerID' field")
		);
		return;
	}

	try
	{
		from_json(gcJ.at("joystickID"), gc.joystickID);
	}
	catch (const nlohmann::json::exception& err)
	{
		ctx.errors.emplace_back(
			MAKE_ERROR_FMT("Error parsing joystickID field for GameControllerState: {}", err.what())
		);
		return;
	}

	if (gc.joystickID == GameController::kInvalidJoystickID)
	{
		return;
	}
	assert(gc.joystickID >= 0);

	gc.joystickID = ctx.gameControllerHandler.GetFirstFreeJoystickID();
	if (gc.joystickID == GameController::kInvalidJoystickID)
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("No free JoystickID found for entity with active GameControllerState")
		);
	}
}

template <typename BasicJson>
void from_json(const BasicJson& j, AudioDeserializerContext& ctx)
{
	if (!j.contains("ActiveAudio"))
	{
		return;
	}

	auto& activeAudioJ = j.at("ActiveAudio");

	if (!activeAudioJ.contains("name"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("ActiveAudio JSON must contain 'name' field")
		);
		return;
	}

	std::string audioName;
	try
	{
		from_json(activeAudioJ.at("name"), audioName);
	}
	catch (const nlohmann::json::exception& err)
	{
		ctx.errors.emplace_back(
			MAKE_ERROR_FMT("Error parsing name field for ActiveAudio: {}", err.what())
		);
		return;
	}

	const auto audioHandle = ctx.audioBank.GetAudio(audioName);
	if (!audioHandle.IsValid())
	{
		return;
	}

	AudioChannelSettings settings{};

	if (!activeAudioJ.contains("settings"))
	{
		ctx.errors.emplace_back(
			MAKE_ERROR("ActiveAudio JSON did not contain 'settings' field. Using default")
		);
		return;
	}
	else
	{
		try
		{
			from_json(activeAudioJ.at("settings"), settings);
		}
		catch (const nlohmann::json::exception& err)
		{
			ctx.errors.emplace_back(
				MAKE_ERROR_FMT("Error parsing settings field for ActiveAudio: {}", err.what())
			);
			return;
		}
	}

	ctx.entity.AddComponent(NewAudioRequest{
		.audioHandle = audioHandle,
		.settings = std::move(settings)
	});
}

namespace {

auto GetDeserializeComponentLambda(const nlohmann::json& entityJ, Entity& e, EntityPassKey key)
{
	return [&entityJ, &e, key]<typename T>(std::string_view cmpName) -> Result<Void> {
		if (!entityJ.contains(cmpName))
		{
			return kVoid;
		}
		try
		{
			from_json(entityJ.at(cmpName), e.AddComponent<T>(key));
		}
		catch (const nlohmann::json::exception& err)
		{
			return MAKE_ERROR_FMT("Could not deserialize component '{}': {}", cmpName, err.what());
		}

		return kVoid;
	};
}

} // unnamed

#define DESERIALIZE_BASIC_COMPONENT(dcLambda, cmpType) do { \
	if (auto res = dcLambda.template operator()<cmpType>(STR(cmpType)); !res.Success()) { \
		deserializationErrors_.emplace_back(res.GetError()); \
	} \
} while (0)

void EntityDeserializer::UpdateContexts(Entity& e)
{
	relationContext_.entity = e;
	physicsContext_.entity = e;
	spriteContext_.entity = e;
	textContext_.entity = e;
	gameControllerContext_.entity = e;
	audioContext_.entity = e;
}

void EntityDeserializer::DeserializeContextComponents(const nlohmann::json& entityJ)
{
	from_json(entityJ, relationContext_);
	from_json(entityJ, physicsContext_);
	from_json(entityJ, spriteContext_);
	from_json(entityJ, textContext_);
	from_json(entityJ, gameControllerContext_);
	from_json(entityJ, audioContext_);
}

void EntityDeserializer::DeserializeBasicComponents(const nlohmann::json& entityJ, Entity& e)
{
	auto dc = GetDeserializeComponentLambda(entityJ, e, GetEntityPassKey());

	DESERIALIZE_BASIC_COMPONENT(dc, Name);
	DESERIALIZE_BASIC_COMPONENT(dc, Transform);
	DESERIALIZE_BASIC_COMPONENT(dc, CameraTarget);
	DESERIALIZE_BASIC_COMPONENT(dc, EntityFlags);
	DESERIALIZE_BASIC_COMPONENT(dc, SpriteAnimationComponent);
	DESERIALIZE_BASIC_COMPONENT(dc, Tags);
}

void EntityDeserializer::DeserializeUserComponents(const nlohmann::json& entityJ, Entity& e)
{
	auto res = ECS::DeserializeUserComponents(entityJ, e); 
	if (!res.Success())
	{
		deserializationErrors_.emplace_back(
			MAKE_ERROR_FMT("Error deserializing user-defined components for entity {}: {}", 
						   e.GetID(), res.GetError().GetMessage())
		);
	}
}

std::vector<Error> EntityDeserializer::DeserializeEntities(const std::string& jsonFilepath)
{
	auto jResult = LoadJson(jsonFilepath);
	if (!jResult.Success())
	{
		return { jResult.GetError() };
	}
	auto& j = jResult.GetValue();

	if (!j.contains("entities") || !j.at("entities").is_array())
	{
		return { MAKE_ERROR_FMT("Entities JSON file must contain an array field named 'entities'") };
	}

	for (auto& entityJ : j.at("entities"))
	{ 
		auto e = ECS::CreateEntity();

		UpdateContexts(e);

		DeserializeBasicComponents(entityJ, e);
		DeserializeContextComponents(entityJ);
		DeserializeUserComponents(entityJ, e);
	}

	ResolveRelations();
	ResolvePhysics();

	return deserializationErrors_;
}

std::vector<Error> EntityDeserializer::DeserializeEntitiesFromJson(const nlohmann::json& masterJ)
{
	if (!masterJ.contains("entities") || !masterJ.at("entities").is_array())
	{
		return { MAKE_ERROR_FMT("Master JSON must contain an array field named 'entities'") };
	}

	for (auto& entityJ : masterJ.at("entities"))
	{
		auto e = ECS::CreateEntity();

		UpdateContexts(e);

		DeserializeBasicComponents(entityJ, e);
		DeserializeContextComponents(entityJ);
		DeserializeUserComponents(entityJ, e);
	}

	ResolveRelations();
	ResolvePhysics();

	return deserializationErrors_;
}

void EntityDeserializer::ResolvePhysics()
{
	for (auto& [childEnt, parentBodyId, colliderBuilder] : physicsContext_.childCollidersToResolve)
	{
		auto it = physicsContext_.bodyIdToEntityMap.find(parentBodyId);
		if (it == physicsContext_.bodyIdToEntityMap.end())
		{
			deserializationErrors_.emplace_back(
				MAKE_ERROR_FMT("Could not resolve parent body for Collider component, "
							   "parentBodyId {} not found", parentBodyId)
			);

			continue;
		}

		auto& parentEnt = it->second;
		assert(parentEnt.IsValid());

		if (!parentEnt.GetRelations().IsParentOf(childEnt))
		{
			deserializationErrors_.emplace_back(
				MAKE_ERROR_FMT("Could not resolve parent body for Collider component, "
							   "entity {} with bodyId {} is not parent of entity {}", 
							   parentEnt.GetID(), parentBodyId, childEnt.GetID())
			);

			continue;
		}

		childEnt.AddComponent(colliderBuilder.Build(parentEnt.GetComponent<RigidBody>().body));
	}
}

void EntityDeserializer::ResolveRelations()
{
	for (auto& child : relationContext_.childrenToResolve)
	{
		assert(child.HasComponent<Parent>());
		auto& parentId = child.GetComponent<Parent>(GetEntityPassKey()).entityId;

		auto it = relationContext_.oldIdsToParentEntities.find(parentId);
		if (it == relationContext_.oldIdsToParentEntities.end())
		{
			deserializationErrors_.emplace_back(
				MAKE_ERROR_FMT("Could not resolve parent-child relationship for entity {},"
							   " parent with old id {} not found", child.GetID(), parentId)
			);

			continue;
		}

		auto& parentEnt = it->second;
		assert(parentEnt.IsValid());

		parentId = parentEnt.GetID();

		auto [_, inserted] = 
			parentEnt.AddComponent<Children>(GetEntityPassKey()).childEntityIds.insert(child.GetID());

		if (!inserted)
		{
			deserializationErrors_.emplace_back(
				MAKE_ERROR_FMT("Could not resolve parent-child relationship for entity {},"
							   " parent with old id {} already has child with id {}", 
							   child.GetID(), parentEnt.GetID(), child.GetID())
			);
		}
	}
}