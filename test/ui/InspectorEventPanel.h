#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../Fixtures.h"
#include "../../core/commonObjects.h"
#include "../../core/Bitset.h"
#include "InspectorCommon.h"
#include "gui_edit/GuiEventNames.h"
#include "gui_edit/GuiEditIncludes.h"

class EventBus;
class TextureRepository;

namespace ui {

class InspectorEventPanel
{
public:
	template <typename T>
	using GuiEditEventPred = std::bool_constant<HasGuiEditProperty<T> && HasGuiEventName<T>>;

	using GuiEventTypeList = filter_types_t<EventDataTypeList, GuiEditEventPred>;

	struct Buttons
	{
		SingleSpriteButton<GuiEventTypeList> fire;
	};

	struct ResourceContext
	{
		EventBus& eventBus;
		TextureRepository& textureRepo;
	};

	static Result<Void> Init(SceneFixture& fixture);

	static void Update(ResourceContext& ctx);

	static Result<Void> ResetForNewScene(SceneFixture& fixture);

	static Buttons& GetButtons() { return buttons_; }

	static void TearDown();

private:
	InspectorEventPanel() = delete;

	static Result<Void> LoadResources(SceneFixture& fixture);

	static inline Buttons buttons_{};
	static inline TypeIndexedBitset<GuiEventTypeList> eventFiredList_{};
	static inline SignalTokenStorage eventFiredTokens_{};
	static inline GuiEventTypeList::AsTuple<std::type_identity_t> editedEvents_{};
};

} // ui

#endif