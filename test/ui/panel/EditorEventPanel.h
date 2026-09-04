#pragma once
#include "IEditorPanel.h"

#if IMGUI_ENABLED
#include "../../../core/commonObjects.h"
#include "../../../core/Bitset.h"
#include "../InspectorCommon.h"
#include "../gui_edit/GuiEventNames.h"
#include "../gui_edit/GuiEditIncludes.h"

class EventBus;
class TextureRepository;

namespace ui {

class EditorEventPanel : public BaseEditorPanel<EditorEventPanel, PanelType::Events>
{
public:
	friend class BaseEditorPanel<EditorEventPanel, PanelType::Events>;

	template <typename T>
	using GuiEditEventPred = std::bool_constant<HasGuiEditProperty<T>&& HasGuiEventName<T>>;

	using GuiEventTypeList = filter_types_t<EventDataTypeList, GuiEditEventPred>;

	struct Buttons
	{
		Button<GuiEventTypeList> fire;
	};

	//EditorEventPanel() : IEditorPanel(PanelType::Events) {}
	//~EditorEventPanel() override = default;

	//void Update(Entity, SceneFixture& fixture) override;
	//Result<Void> Init(SceneFixture& fixture) override;
	//void TearDown() override;

private:
	void UpdateImpl(Entity, SceneFixture& fixture);
	Result<Void> InitImpl(SceneFixture& fixture);
	void ClearStateImpl();
	void TearDownImpl();

	Result<Void> LoadResources(SceneFixture& fixture);

	Buttons buttons_{};
	TypeIndexedBitset<GuiEventTypeList> eventFiredList_{};
	SignalTokenStorage eventFiredTokens_{};
	GuiEventTypeList::AsTuple<std::type_identity_t> editedEvents_{};
};

} // ui

#endif