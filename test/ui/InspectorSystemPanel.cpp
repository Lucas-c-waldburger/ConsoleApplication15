#include "InspectorSystemPanel.h"

#if IMGUI_ENABLED
#include "../../file/FilePathUtility.h"
#include "GuiResource.h"
#include "GuiTexture.h"
#include "InspectorComponentPanel.h"

namespace ui {

namespace {

template <HasGuiSystemName T>
bool DrawPlayPauseButton(T& sys, const GuiTextureConverter& converter)
{
	GuiTexture ppTexture{};
	auto& button = InspectorSystemPanel::GetButtons().playPause;

	static constexpr bool disable = !SomePausable<T>;
	if constexpr (disable)
	{
		ppTexture = converter.FromSprite(button.defaultSprite);
	}
	else
	{
		ppTexture = converter.FromSprite((sys.IsPaused()
			? button.activatedSprite
			: button.defaultSprite));
	}

	assert(ppTexture.textureId != 0);

	bool changed = false;

	ImGui::BeginDisabled(disable);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered.Test<T>() ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton(GuiSystemName<T>::label.data(), 
										ppTexture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	ImGui::EndDisabled();

	button.isHovered.Set<T>(ImGui::IsItemHovered());

	if (pressed)
	{
		if constexpr (SomePausable<T>)
		{
			sys.SetPaused(!sys.IsPaused());
			changed = true;
		}
	}

	return changed;
}

template <typename T>
struct PausableSystemPred : std::bool_constant<SomePausable<T>> {};

using PausableSystemTypeList = filter_types_t<CoreSystemTypeList, PausableSystemPred>;

template <typename TList>
struct draw_systems;

template <typename...Ts>
struct draw_systems<TypeList<Ts...>>
{
	static bool call(SystemManager& sysManager, const TextureRepository& repo)
	{
		static constexpr auto draw = []<typename T>
		(SystemManager& sysManager, const GuiTextureConverter& converter) 
		{
			if (!ImGui::CollapsingHeader(GuiSystemName<T>::name.data(), ImGuiTreeNodeFlags_SpanFullWidth))
			{
				return false;
			}

			if (!sysManager.IsSystemRegistered<T>())
			{
				return false;
			}
			auto& sys = sysManager.GetSystem<T>();

			ImGui::BeginGroup();

			bool changed = DrawPlayPauseButton<T>(sys, converter);

			ImGui::EndGroup();

			return changed;
		};

		GuiTextureConverter converter{ repo };

		bool changed = false;
		((changed |= (draw.template operator()<Ts>(sysManager, converter))), ...);

		return changed;
	}
};

bool DrawSystems(SystemManager& sysManager, const TextureRepository& repo)
{
	return draw_systems<InspectorSystemPanel::NamedSystemTypeList>::call(sysManager, repo);
}

} // unnamed

auto InspectorSystemPanel::ResourceContext::Create(SceneFixture::SharedPtr& scene) -> ResourceContext
{
	return {
		.systemManager = scene->GetSystemManager(),
		.textureRepo = scene->GetTextureRepository()
	};
}

void InspectorSystemPanel::Update(ResourceContext& ctx)
{
	DrawSystems(ctx.systemManager, ctx.textureRepo);
}

Result<Void> InspectorSystemPanel::ResetForNewScene(SceneFixture& scene)
{
	const auto& spriteAtlas = scene.GetTextureRepository().GetSpriteAtlas();
	buttons_.playPause.defaultSprite = spriteAtlas.GetSprite("pause_circle.png");
	buttons_.playPause.activatedSprite = spriteAtlas.GetSprite("play_circle.png");

	return kVoid;
}

Result<Void> InspectorSystemPanel::LoadResources(SceneFixture& scene)
{
	TRY(ResourcePath::Sprite("ui/editor/play_circle.png"), playCirclePath);
	TRY(ResourcePath::Sprite("ui/editor/pause_circle.png"), pauseCirclePath);

	auto& spriteAtlas = scene.GetTextureRepository().GetSpriteAtlas();

	TRY_ASSIGN(buttons_.playPause.defaultSprite, spriteAtlas.LoadSprite(
		scene.GetRenderer(), { .filepath = std::move(pauseCirclePath) }));
	TRY_ASSIGN(buttons_.playPause.activatedSprite, spriteAtlas.LoadSprite(
		scene.GetRenderer(), { .filepath = std::move(playCirclePath) }));

	return kVoid;
}

Result<Void> InspectorSystemPanel::Init(SceneFixture& scene)
{
	LoadResources(scene);

	return kVoid;
}

} // ui

#endif