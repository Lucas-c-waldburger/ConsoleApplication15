#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../Fixtures.h"
#include "../../ecs/Ecs.h"
#include "SpritePicker.h"
#include "InspectorCommon.h"
#include "gui_edit/GuiEditIncludes.h"
#include "gui_edit/GuiEditPropertyTable.h"
#include "gui_edit/GuiComponentNames.h"
#include "gui_edit/GuiEditComponentBuilders.h"

namespace ui {

struct EventCallbackSpoofComponent;

class InspectorComponentPanel
{
public:
	template <typename T>
	using GuiNamedPred = std::bool_constant<HasGuiComponentName<T>>;

	using GuiNamedComponentTypeList = filter_types_t<CoreComponentTypeList, GuiNamedPred>;

	template <typename T>
	using GuiEditablePred = std::bool_constant<HasGuiEditProperty<T> && HasGuiComponentName<T>>;

	using GuiEditableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiEditablePred>;

	template <typename T>
	using GuiBuilderPred = std::bool_constant<HasGuiEditComponentBuilder<T>>;

	using GuiBuilderComponentTypeList = filter_types_t<CoreComponentTypeList, GuiBuilderPred>;

	template <typename T>
	using GuiAddablePred = std::bool_constant<public_mutable_component_v<T> && HasGuiComponentName<T> 
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

	struct ResourceContext
	{
		Entity entity;
		TextureRepository& textureRepo;
		B2World& world;
		ScriptSystem& scriptSys;
		EventBus& eventBus;
	};

	enum UpdateReport : uint8_t
	{
		None = 0,
		HistoryCursorMoved = 1 << 0
	};

	static Result<Void> Init(SceneFixture& scene);

	static UpdateReport Update(ResourceContext& ctx);

	static void ClearState();

	static Buttons& GetButtons() { return buttons_; }
	static SpritePicker& GetSpritePicker() { return spritePicker_; }
	static const ComponentBuilderType& GetActiveBuilderType() { return activeBuilderType_; }
	static void SetActiveBuilderType(ComponentBuilderType type) { activeBuilderType_ = type; }
	static EditableComponentBitSet& GetComponentHeaderOpen() { return componentHeaderOpen_; }

	static Result<Void> ResetForNewScene(SceneFixture& scene);

private:
	static Result<Void> LoadResources(SceneFixture& scene);

	static inline SpritePicker spritePicker_{};
	static inline Buttons buttons_{};
	static inline ComponentBuilderType activeBuilderType_ = ComponentBuilderType::None;
	static inline EditableComponentBitSet componentHeaderOpen_{};

	InspectorComponentPanel() = default;
};

inline constexpr InspectorComponentPanel::UpdateReport& operator|=(
	InspectorComponentPanel::UpdateReport& lhs, InspectorComponentPanel::UpdateReport rhs)
{
	lhs = static_cast<InspectorComponentPanel::UpdateReport>(lhs | rhs);
	return lhs;
}

} // ui

#endif