#pragma once
#include "EditorRigidBodyBuilder.h"
#include "EditorColliderBuilder.h"
#include "EditorEventCallbackBuilder.h"

#if IMGUI_ENABLED
#include <unordered_map>
#include <array>
#include <memory>
#include <magic_enum/magic_enum.hpp>

namespace ui {

class EditorComponentBuilderManager
{
public:
	using BuilderArray = std::array<std::unique_ptr<IEditorComponentBuilder>,
						 magic_enum::enum_count<ComponentBuilderType>()>;

	bool DrawActiveBuilder(Entity& e, SceneFixture& fixture);

	void SetActiveBuilder(ComponentBuilderType type);
	void ClearActiveBuilder();

	ComponentBuilderType GetActiveBuilderType() const noexcept { return activeBuilderType_; }
	bool HasActiveBuilder() const noexcept;

	Result<Void> Init(SceneFixture& fixture);

private:
	BuilderArray builders_;
	ComponentBuilderType activeBuilderType_ = kInvalidComponentBuilderType;
};

} // ui

#endif