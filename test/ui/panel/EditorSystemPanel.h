#pragma once
#include "IEditorPanel.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../../Fixtures.h"
#include "../../../core/commonObjects.h"
#include "../InspectorCommon.h"
#include "../gui_edit/GuiSystemNames.h"

namespace ui {

class EditorSystemPanel : public BaseEditorPanel<EditorSystemPanel, PanelType::Systems>
{
public:
	friend class BaseEditorPanel<EditorSystemPanel, PanelType::Systems>;

	template <typename T>
	struct NamedSystemPred : std::bool_constant<HasGuiSystemName<T>> {};

	using NamedSystemTypeList = filter_types_t<CoreSystemTypeList, NamedSystemPred>;

	struct Buttons
	{
		Button<NamedSystemTypeList> playPause;
		MapButton<ScriptTable::TableId> scriptReload;
		MapButton<ScriptTable::TableId> scriptDelete;
	};

	//EditorSystemPanel() : IEditorPanel(PanelType::Systems) {}
	//~EditorSystemPanel() override = default;

	//void Update(Entity, SceneFixture& fixture) override;
	//Result<Void> Init(SceneFixture& fixture) override;
	//void ClearState() override;

private:
	void UpdateImpl(Entity, SceneFixture& fixture);
	Result<Void> InitImpl(SceneFixture& fixture);
	void ClearStateImpl();
	void TearDownImpl();

	Result<Void> LoadResources(SceneFixture& fixture);

	Buttons buttons_{};
};


} // ui

#endif