#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../Fixtures.h"
#include "../../core/commonObjects.h"
#include "InspectorCommon.h"
#include "gui_edit/GuiSystemNames.h"

namespace ui {

class InspectorSystemPanel
{
public:
	template <typename T>
	struct NamedSystemPred : std::bool_constant<HasGuiSystemName<T>> {};

	using NamedSystemTypeList = filter_types_t<CoreSystemTypeList, NamedSystemPred>;

	struct Buttons
	{
		Button<NamedSystemTypeList> playPause;
	};

	struct ResourceContext
	{
		SystemManager& systemManager;
		const TextureRepository& textureRepo;

		static ResourceContext Create(SceneFixture::SharedPtr& scene);
	};

	static void Update(ResourceContext& resourceCtx);

	static Result<Void> Init(SceneFixture& scene);

	static Buttons& GetButtons() { return buttons_; }

	static Result<Void> ResetForNewScene(SceneFixture& scene);

private:
	static Result<Void> LoadResources(SceneFixture& scene);

	static inline Buttons buttons_{};
};


} // ui

#endif