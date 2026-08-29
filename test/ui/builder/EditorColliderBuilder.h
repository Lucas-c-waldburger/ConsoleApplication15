#pragma once
#include "IEditorComponentBuilder.h"

#if IMGUI_ENABLED
#include "../../../components/builder/ColliderComponentBuilder.h"
#include "../ColliderEditUtility.h"

namespace ui {

class EditorColliderBuilder final : public IEditorComponentBuilder
{
public:
	EditorColliderBuilder() : IEditorComponentBuilder(ComponentBuilderType::Collider) {}
	~EditorColliderBuilder() override = default;

	bool Draw(Entity& e, SceneFixture& fixture) override;

private:
	void SetIsActiveImpl(bool val) override;

	B2ShapeParameters shapeParams_{ .shapeType = B2Shape::Type::Polygon };
	ColliderSettings colliderSettings_{};
	ColliderEditUtility colliderEditUtility_;
};


} // ui

#endif