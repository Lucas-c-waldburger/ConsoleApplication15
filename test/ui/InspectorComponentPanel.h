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
	using GuiAddablePred = std::bool_constant<public_mutable_component_v<T> && HasGuiComponentName<T>>;

	using GuiAddableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiAddablePred>;

	template <typename T>
	using GuiRemovablePred = std::bool_constant<public_mutable_component_v<T> && !std::same_as<T, Name>>;

	using GuiRemovableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiRemovablePred>;

	using NamedComponentBitSet = TypeIndexedBitset<GuiNamedComponentTypeList>;

	struct Buttons
	{
		Button<GuiNamedComponentTypeList> remove;
		Button<GuiNamedComponentTypeList> hide;
	};

	struct ResourceContext
	{
		Entity entity;
		TextureRepository& textureRepo;
		B2World& world;
	};

	static Result<Void> Init(SceneFixture::SharedPtr& scene);

	static void Update(ResourceContext& ctx);

	static void ClearState();

	static Buttons& GetButtons() { return buttons_; }
	static SpritePicker& GetSpritePicker() { return spritePicker_; }
	static ComponentBuilderType& GetActiveBuilderType() { return activeBuilderType_; }
	static NamedComponentBitSet& GetComponentHeaderOpen() { return componentHeaderOpen_; }

private:
	static inline SpritePicker spritePicker_{};
	static inline Buttons buttons_{};
	static inline ComponentBuilderType activeBuilderType_ = ComponentBuilderType::None;
	static inline NamedComponentBitSet componentHeaderOpen_{};

	InspectorComponentPanel() = default;
};

} // ui

#endif