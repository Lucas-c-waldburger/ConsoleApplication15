#include "EntitySerializer.h"
#include "../atlas/NewTextureRepository.h"

template <typename BasicJson>
void to_json(BasicJson& j, const SpriteSerializerContext& ctx)
{
	if (!ctx.entity.HasComponent<SpriteRenderableComponent>())
	{
		return;
	}

	const auto& rend = ctx.entity.GetComponent<SpriteRenderableComponent>();
	auto& spriteJ = j["sprite"];
	auto& profileJ = j["profile"];

	if (const auto nm = ctx.spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName>(rend.sprite))
	{
		spriteJ = *nm;
	}

	to_json(profileJ, rend.profile);
}

template <typename BasicJson>
void to_json(BasicJson& j, const TextSerializerContext& ctx)
{
	if (!ctx.entity.HasComponent<TextRenderableComponent>())
	{
		return;
	}

	const auto& rend = ctx.entity.GetComponent<TextRenderableComponent>();
	auto& fontNameJ = j["fontName"];
	auto& fontSizeJ = j["fontSize"];
	auto& textJ = j["text"];
	auto& profileJ = j["profile"];
	auto& fmtJ = j["formatting"]; 

	const auto op = ctx.fontAtlas.GetFontInfo<&FontInfo::fontName, 
										      &FontInfo::fontSize>
										      (rend.writer.resourceHandle);
	if (op.has_value())
	{
		const auto& [nm, sz] = *op;
		fontNameJ = nm;
		fontSizeJ = sz;
	}

	to_json(textJ, rend.writer.text);
	to_json(profileJ, rend.profile);
}

namespace {

auto GetSerializeComponentLambda(nlohmann::json& entityJ, const Entity& e)
{
	return [&entityJ, &e]<typename T>(std::string_view cmpName) -> void {
		if (!e.HasComponent<T>())
		{
			return;
		}

		to_json(entityJ[cmpName], e.GetComponent<T>());
	};
}

} // unnamed

#define SERIALIZE_BASIC_COMPONENT(scLambda, cmpType) do { \
	scLambda.template operator()<cmpType>(STR(cmpType)); \
} while (0)

Result<Void> EntitySerializer::SerializeEntities(const std::string& jsonFilepath)
{
	std::ofstream file(jsonFilepath);
	if (!file)
	{
		return MAKE_ERROR_FMT("Could not open JSON file at path: '{}'", jsonFilepath);
	}

	nlohmann::json j;

	auto& entitiesJ = j["entities"] = nlohmann::json::array();

	auto entities = ECS::GetAllActiveEntities();
	for (const auto& e : entities)
	{
		UpdateContexts(e);

		auto& entityJ = entitiesJ.emplace_back(nlohmann::json::object());

		entityJ["entityId"] = e.GetID();

		SerializeBasicComponents(entityJ, e);
		SerializeContextComponents(entityJ);
	}

	file << std::setw(4) << j;

	return kVoid;
}

void EntitySerializer::UpdateContexts(const Entity& e)
{
	spriteContext_.entity = e;
	textContext_.entity = e;
}

void EntitySerializer::SerializeBasicComponents(nlohmann::json& entityJ, const Entity& e)
{
	auto sc = GetSerializeComponentLambda(entityJ, e);

	SERIALIZE_BASIC_COMPONENT(sc, Transform);
	SERIALIZE_BASIC_COMPONENT(sc, CameraTarget);
	SERIALIZE_BASIC_COMPONENT(sc, EntityFlags);
	SERIALIZE_BASIC_COMPONENT(sc, SpriteAnimationComponent);
	SERIALIZE_BASIC_COMPONENT(sc, Tags);
	SERIALIZE_BASIC_COMPONENT(sc, Parent);
	SERIALIZE_BASIC_COMPONENT(sc, Children);
}

void EntitySerializer::SerializeContextComponents(nlohmann::json& entityJ)
{
	to_json(entityJ, spriteContext_);
	to_json(entityJ, textContext_);
}