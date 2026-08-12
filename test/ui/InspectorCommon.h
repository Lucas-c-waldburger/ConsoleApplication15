#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../core/Bitset.h"
#include "../../atlas/SpriteAtlasCollection.h"
#include "GuiResource.h"
#include "GuiTexture.h"
#include "../../physics/B2Handle.h"
#include "../../ecs/EntityT.h"

class Entity;
class Camera;
struct GlyphCacheData;
struct Collider;
struct RigidBody;
struct SpriteRenderableComponent;
struct CollisionData;

namespace ui {

/** @defgroup Editor Components @{ */
struct InspectorTag {};
struct CallbackInfo
{
	std::vector<std::string_view> eventNames;
	std::vector<std::string> scriptFileNames;
	std::vector<std::string> tableFunctionNames;
};
/** @} */

struct SimpleButton
{
	Sprite sprite;
	bool isHovered = false;
};

template <typename T>
struct MapButton
{
	Sprite sprite;
	std::unordered_map<T, bool> isHovered;
};

template <typename TList>
struct Button
{
	Sprite defaultSprite;
	Sprite activatedSprite;
	TypeIndexedBitset<TList> isHovered;
};

template <typename TList>
struct SingleSpriteButton
{
	Sprite sprite;
	TypeIndexedBitset<TList> isHovered;
};

template <typename T, typename TList>
bool DrawButton(Button<TList>& button, const GuiTexture& texture, 
				std::string_view label, bool disabled)
{
	assert(texture.textureId != 0);

	ImGui::BeginDisabled(disabled);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered.Test<T>() ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton(label.data(), texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	ImGui::EndDisabled();

	button.isHovered.Set<T>(ImGui::IsItemHovered());

	return pressed;
}

void AssignGuiStyles();

bool BeginComponentTable();

void EndComponentTable();

float GetRightAlignButtonStartX(float buttonSize, size_t numButtons);

SDL_FRect ComputeBoundingBoxForGlyphCache(const std::vector<GlyphCacheData>& data);

SDL_FRect GetScreenRectForEntity(const Entity& e, const Camera& cam);

bool HasValidBody(const RigidBody& rb);
bool HasValidShape(const Collider& col);
bool HasValidResourceHandle(const SpriteRenderableComponent& sp);

struct CollisionDataShapeInfo
{
	struct Elem
	{
		std::string label;
		Entity_t entityId = kInvalidEntity;
		Handle<B2Shape> handle;
	};

	Elem current;
	std::vector<Elem> all;
};

bool IsValidCollisionParticipant(const Entity& e);

std::vector<Entity> GetAllColliderEntitiesForRigidBodyEntity(Entity& e);

CollisionDataShapeInfo GetCollisionDataShapeInfo(Entity& e, const CollisionData& data);

} // ui

#endif