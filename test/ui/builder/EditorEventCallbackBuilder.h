#pragma once
#include "IEditorComponentBuilder.h"

#if IMGUI_ENABLED
#include "../../../systems/ScriptSystem.h"

namespace ui {

class EditorEventCallbackBuilder final : public IEditorComponentBuilder
{
public:
	EditorEventCallbackBuilder() : IEditorComponentBuilder(ComponentBuilderType::EventCallback) {}
	~EditorEventCallbackBuilder() override = default;

	bool Draw(Entity& e, SceneFixture& fixture) override;

private:
	void SetIsActiveImpl(bool val) override;

	bool CanAddCallback();
	void UpdateEntityScriptTable(Entity& e, const ScriptSystem& scriptSys);
	void UpdateEntityCallbackInfo(Entity& e, const ScriptSystem& scriptSys);
	void ClearSelections();

	std::string selectedEventName_{};
	ScriptTable::TableId selectedTableId_ = std::numeric_limits<ScriptTable::TableId>::max();
	std::string selectedTableFunction_{};
	Entity_t selectedRelevantEntity_ = kInvalidEntity;
};


} // ui

#endif