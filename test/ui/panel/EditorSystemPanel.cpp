#include "EditorSystemPanel.h"

#if IMGUI_ENABLED
#include "../ScriptLoaderUtility.h"

namespace ui {

namespace {

template <HasGuiSystemName T>
bool DrawPlayPauseButton(T& sys, EditorSystemPanel::Buttons& buttons, const GuiTextureConverter& converter)
{
	GuiTexture texture{};
	auto& button = buttons.playPause;

	static constexpr bool disable = !SomePausable<T>;
	if constexpr (disable)
	{
		texture = converter.FromSprite(button.defaultSprite);
	}
	else
	{
		texture = converter.FromSprite((sys.IsPaused()
			? button.activatedSprite
			: button.defaultSprite));
	}

	assert(texture.textureId != 0);

	auto h = ImGui::GetFrameHeight();
	texture.size.x = h * 1.05f;
	texture.size.y = h * 1.05f;

	bool changed = false;

	ImGui::BeginDisabled(disable);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered.Test<T>() ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton(GuiSystemName<T>::label.data(),
										texture, ImVec4(0, 0, 0, 0), tint);

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

bool DrawScriptButton(std::string_view label, ScriptTable::TableId tableId,
					  MapButton<ScriptTable::TableId>& button,
					  const GuiTextureConverter& converter)
{
	GuiTexture texture = converter.FromSprite(button.sprite);
	assert(texture.textureId != 0);

	auto h = ImGui::GetFrameHeight();
	texture.size.x = h * 1.05f;
	texture.size.y = h * 1.05f;

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	assert(button.isHovered.contains(tableId));

	const auto tint = button.isHovered[tableId] ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton(label.data(), texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	button.isHovered[tableId] = ImGui::IsItemHovered();

	return pressed;
}

template <typename T>
struct PausableSystemPred : std::bool_constant<SomePausable<T>> {};

using PausableSystemTypeList = filter_types_t<CoreSystemTypeList, PausableSystemPred>;

template <typename T>
struct draw_system
{
	static void call(T& sys, EditorSystemPanel::Buttons& buttons, 
					 const GuiTextureConverter& converter)
	{
		if (!BeginComponentTable())
		{
			return;
		}

		EndComponentTable();
	}
};

template <>
struct draw_system<ScriptSystem>
{
	static void call(ScriptSystem& sys, EditorSystemPanel::Buttons& buttons, 
					 const GuiTextureConverter& converter)
	{
		ImGui::Indent(12.0f);

		auto& reloadButtons = buttons.scriptReload;
		auto& deleteButtons = buttons.scriptDelete;
		const auto& tableMap = sys.GetScriptTableMap();

		ScriptTable::TableId removeTableId = std::numeric_limits<ScriptTable::TableId>::max();

		for (const auto& [id, data] : tableMap)
		{
			const auto filename = std::filesystem::path(data.filepath).filename().string();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 6));

			const bool tableOpen = ImGui::TreeNodeEx(filename.c_str(),
				(ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_FramePadding));

			ImGui::PopStyleVar();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

			const float buttonStartX = GetRightAlignButtonStartX(ImGui::GetFrameHeight(), 2);

			ImGui::SameLine(buttonStartX);

			if (!reloadButtons.isHovered.contains(id))
			{
				reloadButtons.isHovered.try_emplace(id, false);
			}
			if (!deleteButtons.isHovered.contains(id))
			{
				deleteButtons.isHovered.try_emplace(id, false);
			}

			const bool reloadScript = DrawScriptButton("reloadScriptBtn", id, reloadButtons, converter);

			ImGui::SameLine();

			const bool deleteScript = DrawScriptButton("deleteScriptBtn", id, deleteButtons, converter);

			ImGui::PopStyleVar();

			if (deleteScript)
			{
				removeTableId = id;

				tableIdToFormattedFunctionStrings_.erase(id);
			}
			else
			{
				if (reloadScript)
				{
					LOG_IF_ERROR(sys.ReloadTable(id));

					ParseFunctionStrings(data.table, sys.GetState());
				}

				if (tableOpen)
				{
					ImGui::Indent(12.0f);

					if (!tableIdToFormattedFunctionStrings_.contains(id))
					{
						ParseFunctionStrings(data.table, sys.GetState());
					}

					if (auto it = tableIdToFormattedFunctionStrings_.find(id);
						it != tableIdToFormattedFunctionStrings_.end())
					{
						for (const auto& fnStr : it->second)
						{
							ImGui::TextWrapped("%s", fnStr.c_str());
						}
					}

					ImGui::Unindent(12.0f);

					ImGui::TreePop();
				}
			}
		}

		if (removeTableId != std::numeric_limits<ScriptTable::TableId>::max())
		{
			reloadButtons.isHovered.erase(removeTableId);
			deleteButtons.isHovered.erase(removeTableId);
			sys.RemoveTable(removeTableId);
		}

		ImGui::Separator();

		if (ImGui::Button("Load"))
		{
			LOG_IF_ERROR(ScriptLoaderUtility::HandleScriptSelection(sys));
		}

		ImGui::Unindent(12.0f);
	}

	static void ParseFunctionStrings(const ScriptTable& table, const LuaStateManager& state)
	{
		if (!table.IsValid())
		{
			return;
		}

		auto fnTableStringsResult =
			LuaFunctionTableParser::ParseLuaFunctionTableStrings(table.Data(), state);

		auto& fnStrings = tableIdToFormattedFunctionStrings_[table.GetTableId()];
		fnStrings.clear();

		if (!fnTableStringsResult.Success())
		{
			LOG_ERROR(fnTableStringsResult.GetError().GetMessage());
		}
		else
		{
			for (auto&& [fnName, argNames] : std::move(fnTableStringsResult).GetValue())
			{
				auto& formatted = fnStrings.emplace_back();

				formatted = std::move(fnName) + '(';

				for (size_t i = 0; i < argNames.size(); ++i)
				{
					formatted += std::move(argNames[i]);
					if (i < argNames.size() - 1)
					{
						formatted += ", ";
					}
				}

				formatted += ')';
			}
		}
	}

	static inline std::unordered_map<ScriptTable::TableId, std::vector<std::string>>
	tableIdToFormattedFunctionStrings_{};
};

template <typename T>
void DrawSystem(T& sys, EditorSystemPanel::Buttons& buttons, const GuiTextureConverter& converter)
{
	return draw_system<T>::call(sys, buttons, converter);
}

template <typename TList>
struct draw_systems;

template <typename...Ts>
struct draw_systems<TypeList<Ts...>>
{
	static bool call(SystemManager& sysManager, EditorSystemPanel::Buttons& buttons,
					 const TextureRepository& repo)
	{
		static constexpr auto draw = []<typename T>
			(SystemManager& sysManager, EditorSystemPanel::Buttons & buttons, 
			 const GuiTextureConverter & converter)
		{
			if (!sysManager.IsSystemRegistered<T>())
			{
				return false;
			}

			auto& sys = sysManager.GetSystem<T>();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 6));

			const bool open = ImGui::CollapsingHeader(GuiSystemName<T>::name.data(),
													  ImGuiTreeNodeFlags_AllowOverlap);

			ImGui::PopStyleVar();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

			const float buttonStartX = GetRightAlignButtonStartX(ImGui::GetFrameHeight(), 1);

			ImGui::SameLine(buttonStartX);

			bool changed = DrawPlayPauseButton<T>(sys, buttons, converter);

			ImGui::PopStyleVar();

			if (!open)
			{
				return changed;
			}

			DrawSystem(sys, buttons, converter);

			return changed;
		};

		GuiTextureConverter converter{ repo };

		bool changed = false;
		((changed |= (draw.template operator()<Ts>(sysManager, buttons, converter))), ...);

		return changed;
	}
};

bool DrawSystems(SystemManager& sysManager, EditorSystemPanel::Buttons& buttons, 
				 const TextureRepository& repo)
{
	return draw_systems<EditorSystemPanel::NamedSystemTypeList>::call(sysManager, buttons, repo);
}

} // unnamed

//void EditorSystemPanel::Update(Entity, SceneFixture& fixture)
//{
//	auto& auxRepo = fixture.GetAuxTextureRepository();
//	assert(auxRepo);
//
//	DrawSystems(fixture.GetSystemManager(), buttons_, *auxRepo);
//}
//
//Result<Void> EditorSystemPanel::LoadResources(SceneFixture& fixture)
//{
//	TRY(ResourcePath::Sprite("ui/editor/play_circle.png"), playCirclePath);
//	TRY(ResourcePath::Sprite("ui/editor/pause_circle.png"), pauseCirclePath);
//	TRY(ResourcePath::Sprite("ui/editor/reload_icon.png"), reloadIconPath);
//	TRY(ResourcePath::Sprite("ui/editor/delete_icon.png"), deleteIconPath);
//
//	auto& auxRepo = fixture.GetAuxTextureRepository();
//	if (!auxRepo)
//	{
//		return MAKE_ERROR("Aux TextureRepository was null");
//	}
//	auto& spriteAtlas = auxRepo->GetSpriteAtlas();
//
//	TRY_ASSIGN(buttons_.playPause.defaultSprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(pauseCirclePath) }));
//	TRY_ASSIGN(buttons_.playPause.activatedSprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(playCirclePath) }));
//	TRY_ASSIGN(buttons_.scriptReload.sprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(reloadIconPath) }));
//	TRY_ASSIGN(buttons_.scriptDelete.sprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(deleteIconPath) }));
//
//	TRY(ResourcePath::Script("fn_table.lua"), fnTableScriptPath);
//
//	assert(fixture.IsSystemRegistered<ScriptSystem>());
//	auto& scriptSys = fixture.GetSystem<ScriptSystem>();
//
//	TRY(scriptSys.AddFunctionTable(fnTableScriptPath));
//
//	return kVoid;
//}
//
//Result<Void> EditorSystemPanel::Init(SceneFixture& fixture)
//{
//	TRY(LoadResources(fixture));
//
//	return kVoid;
//}
//
//void EditorSystemPanel::ClearState()
//{
//	draw_system<ScriptSystem>::tableIdToFormattedFunctionStrings_.clear();
//}

void EditorSystemPanel::UpdateImpl(Entity, SceneFixture& fixture)
{
	auto& auxRepo = fixture.GetAuxTextureRepository();
	assert(auxRepo);

	DrawSystems(fixture.GetSystemManager(), buttons_, *auxRepo);
}

Result<Void> EditorSystemPanel::LoadResources(SceneFixture& fixture)
{
	TRY(ResourcePath::Sprite("ui/editor/play_circle.png"), playCirclePath);
	TRY(ResourcePath::Sprite("ui/editor/pause_circle.png"), pauseCirclePath);
	TRY(ResourcePath::Sprite("ui/editor/reload_icon.png"), reloadIconPath);
	TRY(ResourcePath::Sprite("ui/editor/delete_icon.png"), deleteIconPath);

	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Aux TextureRepository was null");
	}
	auto& spriteAtlas = auxRepo->GetSpriteAtlas();

	TRY_ASSIGN(buttons_.playPause.defaultSprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(pauseCirclePath) }));
	TRY_ASSIGN(buttons_.playPause.activatedSprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(playCirclePath) }));
	TRY_ASSIGN(buttons_.scriptReload.sprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(reloadIconPath) }));
	TRY_ASSIGN(buttons_.scriptDelete.sprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(deleteIconPath) }));

	TRY(ResourcePath::Script("fn_table.lua"), fnTableScriptPath);

	assert(fixture.IsSystemRegistered<ScriptSystem>());
	auto& scriptSys = fixture.GetSystem<ScriptSystem>();

	TRY(scriptSys.AddFunctionTable(fnTableScriptPath));

	return kVoid;
}

Result<Void> EditorSystemPanel::InitImpl(SceneFixture& fixture)
{
	TRY(LoadResources(fixture));

	return kVoid;
}

void EditorSystemPanel::ClearStateImpl()
{
	draw_system<ScriptSystem>::tableIdToFormattedFunctionStrings_.clear();
}

void EditorSystemPanel::TearDownImpl() {}


} // ui

#endif