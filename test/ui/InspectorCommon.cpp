#include "InspectorCommon.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include <deque>
#include "../../camera/Camera.h"
#include "../../systems/util/DebugDrawUtils.h"
#include "../../ecs/Ecs.h"
#include "../../components/util/ComponentValidPreds.h"
#include "../../events/data/EntityCollision.h"

namespace ui {

namespace {

void AssignGuiColors()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);

	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.125f, 0.125f, 0.135f, 1.00f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.145f, 0.145f, 0.155f, 1.00f);

	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);

	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.17f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.58f, 1.00f);

	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3f, 0.5f, 0.8f, 0.25f);

	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.65f, 0.70f, 1.00f);

	style.Colors[ImGuiCol_Header] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.19f, 0.19f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
}

void AssignGuiRounding()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.FrameRounding = 4;
	style.WindowRounding = 6;
	style.ScrollbarRounding = 4;
	style.GrabRounding = 4;
}

} // unnamed

void AssignGuiStyles()
{
	AssignGuiRounding();
	AssignGuiColors();
}

bool BeginComponentTable()
{
	if (!ImGui::BeginTable("Component Table", 2,
		ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_NoBordersInBody))
	{
		return false;
	}

	ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
	ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

	return true;
}

void EndComponentTable()
{
	ImGui::EndTable();
}

float GetRightAlignButtonStartX(float buttonSize, size_t numButtons)
{
	assert(numButtons > 0);

	const float spacing = ImGui::GetStyle().ItemSpacing.x;

	const float totalWidth =
		static_cast<float>(numButtons) * buttonSize +
		static_cast<float>(numButtons - 1) * spacing;

	return ImGui::GetWindowContentRegionMax().x
		- totalWidth
		- ImGui::GetStyle().WindowPadding.x;
}

SDL_FRect ComputeBoundingBoxForGlyphCache(const std::vector<GlyphCacheData>& data)
{
	if (data.empty())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	float minX = data[0].destRect.x;
	float minY = data[0].destRect.y;
	float maxX = data[0].destRect.x + data[0].destRect.w;
	float maxY = data[0].destRect.y + data[0].destRect.h;

	for (size_t i = 1; i < data.size(); ++i)
	{
		minX = std::min(minX, data[i].destRect.x);
		minY = std::min(minY, data[i].destRect.y);
		maxX = std::max(maxX, data[i].destRect.x + data[i].destRect.w);
		maxY = std::max(maxY, data[i].destRect.y + data[i].destRect.h);
	}

	return { minX, minY, maxX - minX, maxY - minY };
}

SDL_FRect GetScreenRectForEntity(const Entity& e, const Camera& cam)
{
	if (!e.HasComponent<Transform>())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	const auto& tf = e.GetComponent<Transform>();

	if (e.HasComponent<SpriteRenderableComponent>(&HasValidResourceHandle))
	{
		const auto& rend = e.GetComponent<SpriteRenderableComponent>();
		auto plotRect = rend.sprite.plot.rect;

		float scaledW = static_cast<float>(plotRect.w) * tf.scale.x;
		float scaledH = static_cast<float>(plotRect.h) * tf.scale.y;

		SDL_FRect result{
			tf.position.x - (scaledW / 2.0f),
			tf.position.y - (scaledH / 2.0f),
			scaledW,
			scaledH
		};

		if (!rend.profile.isOverlay)
		{
			result = cam.WorldToScreen<SDL_FRect>(result);
		}

		return result;
	}

	if (e.HasComponent<TextRenderableComponent>())
	{
		if (e.HasComponent<TextRenderableGlyphCache>())
		{
			const auto& rend = e.GetComponent<TextRenderableComponent>();
			const auto& glyphs = e.GetComponent<TextRenderableGlyphCache>();

			auto result = ComputeBoundingBoxForGlyphCache(glyphs.cache);

			if (!rend.profile.isOverlay)
			{
				result = cam.WorldToScreen<SDL_FRect>(result);
			}

			return result;
		}
	}

	if (e.HasComponent<Collider>(&HasValidShape))
	{
		const auto& shape = e.GetComponent<Collider>().shape.GetData();

		return cam.WorldToScreen<SDL_FRect>(shape.GetBoundingBox());
	}

	return {
		tf.position.x - (128.0f / 2.0f),
		tf.position.y - (128.0f / 2.0f),
		128.0f,
		128.0f
	};
}

bool HasValidBody(const RigidBody& rb)
{
	return rb.body.GetData().IsValid();
}

bool HasValidShape(const Collider& col)
{
	return col.shape.GetData().IsValid();
}

bool HasValidResourceHandle(const SpriteRenderableComponent& sp)
{
	return sp.sprite.resourceHandle.IsValid();
}

bool IsValidCollisionParticipant(const Entity& e)
{
	return e.IsValid() && e.HasComponent<Name>() && e.HasComponent<RigidBody>(&RigidBodyValid);
}

std::vector<Entity> GetAllColliderEntitiesForRigidBodyEntity(Entity& e)
{
	if (!e.HasComponent<RigidBody>(&RigidBodyValid))
	{
		return {};
	}

	const auto& body = e.GetComponent<RigidBody>().body.GetData();
	std::vector<Entity> colliderEs;

	if (e.HasComponent<Collider>(&ColliderValid))
	{
		colliderEs.emplace_back(e);
	}

	auto rels = e.GetRelations();
	if (rels.HasChildren())
	{
		auto chs = rels.GetAllChildrenWith<Collider>();
		for (const auto& ch : chs)
		{
			if (ch.GetComponent<Collider>().shape.GetData().GetParentBodyHandle() ==
				body.GetHandle())
			{
				colliderEs.emplace_back(ch);
			}
		}
	}

	return colliderEs;
}

CollisionDataShapeInfo GetCollisionDataShapeInfo(Entity& e, const CollisionData& data)
{
	if (!IsValidCollisionParticipant(e))
	{
		return {};
	}

	auto colEs = GetAllColliderEntitiesForRigidBodyEntity(e);

	std::sort(colEs.begin(), colEs.end(), [](const Entity& a, const Entity& b) {
		const auto shA = a.GetComponent<Collider>().shape.GetData();
		const auto shB = b.GetComponent<Collider>().shape.GetData();

		if (shA.GetShapeType() == shB.GetShapeType())
		{
			return shA.GetHandle() < shB.GetHandle();
		}

		return shA.GetShapeType() < shB.GetShapeType();
	});

	CollisionDataShapeInfo info{};
	info.all.reserve(colEs.size());

	size_t counter = 0;
	B2Shape::Type lastType = B2Shape::Type::Invalid;
	for (const auto& colE : colEs)
	{
		const auto& sh = colE.GetComponent<Collider>().shape.GetData();

		if (sh.GetShapeType() != lastType)
		{
			counter = 0;
		}

		auto& elem = info.all.emplace_back(
			std::format("{} {}", ToString(sh.GetShapeType()), counter),
			colE.GetID(),
			sh.GetHandle()
		);

		if (data.shapeHandle == elem.handle)
		{
			assert(data.entity == elem.entityId);
			info.current = elem;
		}

		++counter;
	}

	return info;
}


} // ui

#endif