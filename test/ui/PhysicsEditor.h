#pragma once

#if IMGUI_ENABLED

#include "../../components/builder/RigidBodyComponentBuilder.h"
#include "../../components/builder/ColliderComponentBuilder.h"
#include "DataEditDisplayUtils.h"

class Entity;

struct PhysicsEditorContext
{
	static inline B2World* world = nullptr;
	static inline std::optional<BodyParameters> bodyParams{};
	static inline std::optional<B2ShapeParameters> shapeParams{};
	static inline std::optional<ColliderSettings> colliderSettings{};
};

class PhysicsEditor
{
public:
	static constexpr const char* kBodyTypeNames[] = {
		"Static", "Kinematic", "Dynamic"
	};
	static constexpr const char* kShapeTypeNames[] = {
		"Circle", "Capsule", "Segment", "Polygon", "ChainSegment"
	};
	 
	static Result<Void> Init(B2World& world);
	static bool IsInitialized() { return PhysicsEditorContext::world != nullptr; }

	static bool DrawBodyParameters(BodyParameters& params);
	static bool DrawBodyLimits(BodyLimits& limits);
	static bool DrawShapeParameters(B2ShapeParameters& params);
	static bool DrawColliderSettings(ColliderSettings& settings);

	static void DrawRigidBodyInfo(const RigidBody& rigidBody);
	static void DrawColliderInfo(const Collider& collider);

	static bool DrawBuildEditor();
	static void BuildOnEntity(Entity& e);

	static void DrawWorldEditor();

private:
	static SimpleGuiTable& GetRigidBodyInfoTable();
	static SimpleGuiTable& GetColliderInfoTable();

	static bool ReadyToBuild();
};

#endif