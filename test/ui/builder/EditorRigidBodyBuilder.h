#pragma once
#include "IEditorComponentBuilder.h"

#if IMGUI_ENABLED
#include "../../../components/builder/RigidBodyComponentBuilder.h"

namespace ui {

class EditorRigidBodyBuilder final : public IEditorComponentBuilder
{
public:
	EditorRigidBodyBuilder() : IEditorComponentBuilder(ComponentBuilderType::RigidBody) {}
	~EditorRigidBodyBuilder() override = default;

	bool Draw(Entity& e, SceneFixture& fixture) override;

private:
	void SetIsActiveImpl(bool val) override;

	BodyParameters bodyParams_{};
	BodyLimits bodyLimits_{};
	bool manuallySelectingPosition_ = false;
};


} // ui

#endif