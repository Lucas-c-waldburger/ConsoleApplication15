#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../InspectorCommon.h"
#include "../../../components/builder/RigidBodyComponentBuilder.h"
#include "../../../components/builder/ColliderComponentBuilder.h"

class B2World;
class Entity;
class ScriptSystem;
class EventBus;

namespace ui {

template <typename T>
class GuiEditComponentBuilder;

enum class ComponentBuilderType
{
	None,
	RigidBody,
	Collider,
	EventCallback
};

template <typename T>
concept HasGuiEditComponentBuilder = requires(bool b) {
	sizeof(GuiEditComponentBuilder<T>);
	{ GuiEditComponentBuilder<T>::IsActive() } -> std::convertible_to<bool>;
	{ GuiEditComponentBuilder<T>::SetIsActive(b) } -> std::same_as<void>;
	{ GuiEditComponentBuilder<T>::GetBuilderType() } -> std::convertible_to<ComponentBuilderType>;
};

template <>
class GuiEditComponentBuilder<RigidBody>
{
public:
	static bool Draw(Entity& e, B2World& world);

	static bool IsActive() { return isActive_; }

	static void SetIsActive(bool active);

	static constexpr ComponentBuilderType GetBuilderType() { return ComponentBuilderType::RigidBody; }

private:
	GuiEditComponentBuilder() = default;

	static inline BodyParameters bodyParams_{};
	static inline BodyLimits bodyLimits_{};

	static inline bool manuallySelectingPosition_ = false;
	static inline bool isActive_ = false;
};

template <>
class GuiEditComponentBuilder<Collider>
{
public:
	static bool Draw(Entity& e, ReadOnly<B2Body>& roBody);

	static bool IsActive() { return isActive_; }

	static void SetIsActive(bool active);

	static const B2ShapeParameters& GetCurrentShapeParameters() { return shapeParams_; }

	static constexpr ComponentBuilderType GetBuilderType() { return ComponentBuilderType::Collider; }

private:
	GuiEditComponentBuilder() = default;

	static inline B2ShapeParameters shapeParams_{ .shapeType = B2Shape::Type::Polygon };
	static inline ColliderSettings colliderSettings_{};

	static inline bool isActive_ = false;
};

template <>
class GuiEditComponentBuilder<CallbackInfo>
{
public:
	static bool Draw(Entity& e, ScriptSystem& scriptSys, EventBus& bus);

	static bool IsActive() { return isActive_; }

	static void SetIsActive(bool active);

	static constexpr ComponentBuilderType GetBuilderType() { return ComponentBuilderType::EventCallback; }

private:
	static bool CanAddCallback();

	static void UpdateEntityScriptTable(Entity& e, const ScriptSystem& scriptSys);
	static void UpdateEntityCallbackInfo(Entity& e);
	static void ClearSelections();

	static inline std::string_view selectedEventName_{};
	static inline std::string selectedScriptFile_{};
	static inline std::string selectedTableFunction_{};
	static inline bool isActive_ = false;
};

} // ui

#endif 