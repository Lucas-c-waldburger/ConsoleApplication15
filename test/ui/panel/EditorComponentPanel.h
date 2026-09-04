#pragma once
#include "IEditorPanel.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"
#include "../../../ecs/Ecs.h"
#include "../SpritePicker.h"
#include "../InspectorCommon.h"
#include "../gui_edit/GuiEditIncludes.h"
#include "../gui_edit/GuiEditPropertyTable.h"
#include "../gui_edit/GuiComponentNames.h"
#include "../gui_edit/GuiEditComponentBuilders.h"
#include "../builder/EditorComponentBuilderManager.h"

namespace ui {

class EditorComponentPanel : public BaseEditorPanel<EditorComponentPanel, PanelType::Components>
{
public:
	friend class BaseEditorPanel<EditorComponentPanel, PanelType::Components>;

	template <typename T>
	using GuiNamedPred = std::bool_constant<HasGuiComponentName<T>>;

	using GuiNamedComponentTypeList = filter_types_t<CoreComponentTypeList, GuiNamedPred>;

	template <typename T>
	using GuiEditablePred = std::bool_constant<HasGuiEditProperty<T>&& HasGuiComponentName<T>>;

	using GuiEditableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiEditablePred>;

	template <typename T>
	using GuiAddablePred = std::bool_constant<public_mutable_component_v<T>&& HasGuiComponentName<T>
		&& HasGuiEditProperty<T>>;

	using GuiAddableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiAddablePred>;

	template <typename T>
	using GuiRemovablePred = std::bool_constant<public_mutable_component_v<T> && !std::same_as<T, Name>>;

	using GuiRemovableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiRemovablePred>;

	using NamedComponentBitSet = TypeIndexedBitset<GuiNamedComponentTypeList>;
	using EditableComponentBitSet = TypeIndexedBitset<GuiEditableComponentTypeList>;

	struct Buttons
	{
		Button<GuiNamedComponentTypeList> remove;
		Button<GuiNamedComponentTypeList> hide;
		SimpleButton undo;
		SimpleButton redo;
	};

	//EditorComponentPanel() : IEditorPanel(PanelType::Components) {}
	//~EditorComponentPanel() override = default;

	//void Update(Entity e, SceneFixture& fixture) override;
	//Result<Void> Init(SceneFixture& fixture) override;
	//void ClearState() override;

	ComponentBuilderType GetActiveBuilderType() { return componentBuilders_.GetActiveBuilderType(); }
	void SetActiveBuilderType(ComponentBuilderType type) { componentBuilders_.SetActiveBuilder(type); }

private:
	void UpdateImpl(Entity e, SceneFixture& fixture);
	Result<Void> InitImpl(SceneFixture& fixture);
	void ClearStateImpl();
	void TearDownImpl();

	Result<Void> LoadResources(SceneFixture& fixture);

	SpritePicker spritePicker_{};
	Buttons buttons_{};
	EditableComponentBitSet componentHeaderOpen_{};
	EditorComponentBuilderManager componentBuilders_{};
};

} // ui

#endif