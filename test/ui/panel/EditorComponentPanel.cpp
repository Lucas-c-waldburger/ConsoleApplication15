#include "EditorComponentPanel.h"

#if IMGUI_ENABLED
#include <format>
#include "../GuiTexture.h"
#include "../InspectorCommon.h"
#include "../../../ecs/EntityPhysics.h"
#include "../GuiMouse.h"
#include "../ComponentEditHistory.h"
#include "../../../components/util/ComponentValidPreds.h"
#include "../ColliderEditUtility.h"

namespace ui {

namespace {

static constexpr std::string_view kNoneName = "<none>";

/** @degroup Add Component @{ */

template <typename TList>
struct draw_add_component_list;

template <typename...Ts>
struct draw_add_component_list<TypeList<Ts...>>
{
	static bool call(Entity& e, EditorComponentPanel::EditableComponentBitSet& componentHeaderOpen)
	{
		static constexpr auto add = []<typename T>(
			Entity& e, EditorComponentPanel::EditableComponentBitSet& componentHeaderOpen) {
			if (e.HasComponent<T>())
			{
				return false;
			}

			if (ImGui::Selectable(GuiComponentName<T>::name.data()))
			{
				const auto& cmp = e.AddComponent<T>();

				ComponentEditHistory::PushAddComponent(e, cmp);

				LOG_DEBUG_FMT("Pushed Add Component : {} (Size: {})",
					GuiComponentName<T>::name, ComponentEditHistory::GetRecordsSize());

				componentHeaderOpen.Set<T>(true);

				return true;
			}

			return false;
		};

		bool changed = false;
		((changed |= (add.template operator()<Ts>(e, componentHeaderOpen))), ...);

		return changed;
	}
};

bool DrawAddComponentOptions(Entity& e, EditorComponentPanel::EditableComponentBitSet& componentHeaderOpen)
{
	bool added = false;

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		added = draw_add_component_list<
			EditorComponentPanel::GuiAddableComponentTypeList>::call(e, componentHeaderOpen);

		if (added)
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return added;
}

/** @} */

/** @degroup Remove Component @{ */

template <typename T>
concept GuiRemovableComponent =
	HasGuiComponentName<T> && public_mutable_component_v<T> && !std::same_as<T, Name>;

template <typename T>
bool DrawRemoveButton(Entity& e, EditorComponentPanel::Buttons& buttons, 
					  const GuiTextureConverter& converter)
{
	auto& button = buttons.remove;

	GuiTexture texture{ converter.FromSprite(button.defaultSprite) };

	auto h = ImGui::GetFrameHeight();
	texture.size.x = h;
	texture.size.y = h;

	assert(texture.textureId != 0);

	ImGui::BeginDisabled(!GuiRemovableComponent<T>);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered.Test<T>() ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const std::string label = std::format("RemoveButton{}", GuiComponentName<T>::name);
	const bool pressed = GuiImageButton(label.c_str(), texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	ImGui::EndDisabled();

	button.isHovered.Set<T>(ImGui::IsItemHovered());

	bool removed = false;
	if (pressed)
	{
		if constexpr (GuiRemovableComponent<T>)
		{
			const auto& cmp = e.GetComponent<T>();

			ComponentEditHistory::PushRemoveComponent(e, cmp);
			LOG_DEBUG_FMT("Pushed Remove Component : {} (Size: {})",
				GuiComponentName<T>::name, ComponentEditHistory::GetRecordsSize());

			e.RemoveComponent<T>();

			button.isHovered.Set<T>(false);
			removed = true;
		}
	}

	return removed;
}

/** @} */

/** @degroup Hide Component @{ */

template <typename T>
concept GuiHideableComponent = HasGuiComponentName<T> &&
	public_mutable_component_v<T> && !std::same_as<T, Name>;

template <typename T>
bool DrawHideButton(Entity& e, EditorComponentPanel::Buttons& buttons, 
					const GuiTextureConverter& converter)
{
	GuiTexture texture{};
	auto& button = buttons.hide;

	if constexpr (!GuiHideableComponent<T>)
	{
		texture = converter.FromSprite(button.defaultSprite);
	}
	else
	{
		texture = converter.FromSprite((e.GetComponentVisibility<T>()
			? button.defaultSprite
			: button.activatedSprite));
	}
	auto h = ImGui::GetFrameHeight();
	texture.size.x = h * 1.05f;
	texture.size.y = h * 1.05f;

	assert(texture.textureId != 0);

	ImGui::BeginDisabled(!GuiHideableComponent<T>);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered.Test<T>() ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const std::string label = std::format("HideButton{}", GuiComponentName<T>::name);
	const bool pressed = GuiImageButton(label, texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	ImGui::EndDisabled();

	button.isHovered.Set<T>(ImGui::IsItemHovered());

	bool changed = false;
	if (pressed)
	{
		if constexpr (GuiHideableComponent<T>)
		{
			e.SetComponentVisibility<T>(!e.GetComponentVisibility<T>());
			changed = true;
		}
	}

	return changed;
}

/** @} */

/** @degroup Undo/Redo @{ */

bool DrawUndoRedoButtonImpl(const GuiTextureConverter& converter, SimpleButton& button,
							const char* label, bool beginDisabled)
{
	GuiTexture texture{ converter.FromSprite(button.sprite) };
	auto h = ImGui::GetFrameHeight();
	texture.size.x = h * 1.05f;
	texture.size.y = h * 1.05f;

	assert(texture.textureId != 0);

	ImGui::BeginDisabled(beginDisabled);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton(label, texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	ImGui::EndDisabled();

	button.isHovered = ImGui::IsItemHovered();

	return pressed;
}

bool DrawUndoButton(const GuiTextureConverter& converter, EditorComponentPanel::Buttons& buttons)
{
	return DrawUndoRedoButtonImpl(converter, buttons.undo, "UndoButton", !ComponentEditHistory::CanUndo());
}

bool DrawRedoButton(const GuiTextureConverter& converter, EditorComponentPanel::Buttons& buttons)
{
	return DrawUndoRedoButtonImpl(converter, buttons.redo, "RedoButton", !ComponentEditHistory::CanRedo());
}

void HandleUndoRedoButtons(const GuiTextureConverter& converter, EditorComponentPanel::Buttons& buttons)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

	const float buttonStartX = GetRightAlignButtonStartX(ImGui::GetFrameHeight(), 2);

	ImGui::SameLine(buttonStartX);

	if (DrawUndoButton(converter, buttons))
	{
		const int cursor = ComponentEditHistory::GetCursor();

		ComponentEditHistory::Undo();

		LOG_DEBUG_FMT("Undo : {} -> {} (Size: {})", cursor,
			ComponentEditHistory::GetCursor(), ComponentEditHistory::GetRecordsSize());
	}

	ImGui::SameLine();

	if (DrawRedoButton(converter, buttons))
	{
		const int cursor = ComponentEditHistory::GetCursor();

		ComponentEditHistory::Redo();

		LOG_DEBUG_FMT("Redo : {} -> {} (Size: {})", cursor,
			ComponentEditHistory::GetCursor(), ComponentEditHistory::GetRecordsSize());
	}

	ImGui::PopStyleVar();
}

/** @} */

/** @degroup Draw Component @{ */

struct DrawComponentReport
{
	PropertyEditState editState = PropertyEditState::None;
	ComponentBuilderType openBuilder = kInvalidComponentBuilderType;

	DrawComponentReport& operator|=(const DrawComponentReport& rhs)
	{
		editState |= rhs.editState;
		if (openBuilder == kInvalidComponentBuilderType)
		{
			openBuilder = rhs.openBuilder;
		}

		return *this;
	}
};

template <typename T>
struct draw_component : EntityFullAccessPrivelage
{
	static DrawComponentReport call(Entity& e, TextureRepository&, SpritePicker&)
	{
		PushPropertyDepth();

		DrawComponentReport report{
			.editState = Property("", e.GetComponent<T>(GetEntityPassKey()))
		};

		PopPropertyDepth();

		return report;
	}
};

template <>
struct draw_component<SpriteRenderableComponent>
{
	static DrawComponentReport call(Entity& e, TextureRepository& repo, SpritePicker& spritePicker)
	{
		assert(e.HasComponent<SpriteRenderableComponent>());
		auto& r = e.GetComponent<SpriteRenderableComponent>();

		DrawComponentReport report{};

		report.editState = PropertyGroup("sprite", [&] {
			auto st = PropertyEditState::None;

			std::string name = repo.GetSpriteAtlas().GetSpriteInfo<&SpriteInfo::spriteName>(r.sprite)
				.value_or(std::string{ kNoneName });

			bool showPicker = false;

			Property("name", [&name, &showPicker] {
				ImGui::SameLine();
				showPicker = ImGui::Button("Browse");
				ImGui::SameLine();
				GuiEditProperty(name);

				return PropertyEditState::None;
			});
			if (showPicker)
			{
				ImGui::OpenPopup("DrawPickerPopup");
				st = PropertyEditState::Started;
			}

			if (ImGui::BeginPopup("DrawPickerPopup"))
			{
				const bool spriteSelected = spritePicker.Draw(repo);
				if (spriteSelected)
				{
					auto spriteData = spritePicker.GetSelectedSprite();
					assert(spriteData.has_value());
					assert(spriteData->sprite.resourceHandle.IsValid());

					r.sprite = std::move(spriteData->sprite);

					spritePicker.Reset();

					assert(st != PropertyEditState::Started);
					st = PropertyEditState::Finished;

					ImGui::CloseCurrentPopup();
				}
				else if (st != PropertyEditState::Started)
				{
					st = PropertyEditState::Active;
				}

				ImGui::EndPopup();
			}

			return st;
		});

		report.editState |= PropertyGroup("profile", [&] { return Property("", r.profile); });

		return report;
	}
};

template <>
struct draw_component<RigidBody>
{
	static DrawComponentReport call(Entity& e, TextureRepository&, SpritePicker&)
	{
		assert(e.HasComponent<RigidBody>());
		auto& rb = e.GetComponent<RigidBody>();

		DrawComponentReport report{};

		PushPropertyDepth();

		if (rb.body.GetData().IsValid())
		{
			report.editState |= Property("", rb);
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button("Build"))
		{
			report.openBuilder = ComponentBuilderType::RigidBody;
		}

		PopPropertyDepth();

		return report;
	}
};

template <>
struct draw_component<Collider>
{
	static DrawComponentReport call(Entity& e, TextureRepository&, SpritePicker&)
	{
		assert(e.HasComponent<Collider>());
		auto& col = e.GetComponent<Collider>();

		DrawComponentReport report{};

		PushPropertyDepth();

		if (col.shape.GetData().IsValid())
		{
			report.editState |= Property("", col);
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::BeginDisabled(!e.HasComponent<RigidBody>(&RigidBodyValid));

		if (ImGui::Button("Build"))
		{
			report.openBuilder = ComponentBuilderType::Collider;
		}

		ImGui::EndDisabled();

		PopPropertyDepth();

		return report;
	}
};

template <>
struct draw_component<SignalTokenStorage>
{
	static DrawComponentReport call(Entity& e, TextureRepository&, SpritePicker&)
	{
		PushPropertyDepth();

		DrawComponentReport report{};

		auto& cbInfo = e.AddComponent<CallbackInfo>();
		assert(cbInfo.eventNames.size() == cbInfo.scriptFileNames.size());
		assert(cbInfo.eventNames.size() == cbInfo.tableFunctionNames.size());

		CallbackInfoEntryContext cbInfoCtx{
			.cbInfo = cbInfo
		};

		const auto& tks = e.GetComponent<SignalTokenStorage>();

		for (size_t i = 0; i < cbInfo.eventNames.size(); ++i)
		{
			const std::string label = std::format("{}##{}", cbInfo.eventNames[i], i);

			cbInfoCtx.i = i;
			const auto& constCbInfoCtx = cbInfoCtx;

			PropertyGroup(label, [&] {
				return Property("", constCbInfoCtx);
			});
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button("Build"))
		{
			report.openBuilder = ComponentBuilderType::EventCallback;
		}

		PopPropertyDepth();

		return report;
	}
};

template <typename T>
DrawComponentReport DrawComponent(Entity& e, TextureRepository& repo, SpritePicker& spritePicker)
{
	return draw_component<T>::call(e, repo, spritePicker);
}

/** @} */

/** @defgroup Draw Components @{ */

struct DrawComponentsContext
{
	Entity& e;
	EditorComponentPanel::Buttons& buttons;
	EditorComponentPanel::EditableComponentBitSet& componentHeaderOpen;
	TextureRepository& primaryRepo;
	SpritePicker& spritePicker;
	GuiTextureConverter converter;
};

template <typename TList>
struct draw_components;

template <template <typename...> class TList, typename...Ts>
struct draw_components<TList<Ts...>> : EntityFullAccessPrivelage
{
	static DrawComponentReport call(DrawComponentsContext& ctx)
	{
		static constexpr auto draw = []<typename T>(DrawComponentsContext& ctx) -> DrawComponentReport
		{
			if (!ctx.e.HasComponent<T>())
			{
				return {};
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 6));

			int headerFlags = ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DrawLinesFull;
			if (ctx.componentHeaderOpen.Test<T>())
			{
				headerFlags |= ImGuiTreeNodeFlags_DefaultOpen;
			}

			const bool open = ImGui::CollapsingHeader(GuiComponentName<T>::name.data(), headerFlags);

			ImGui::PopStyleVar();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

			const float buttonStartX = GetRightAlignButtonStartX(ImGui::GetFrameHeight(), 2);

			ImGui::SameLine(buttonStartX);

			DrawHideButton<T>(ctx.e, ctx.buttons, ctx.converter);

			ImGui::SameLine();

			const bool removed = DrawRemoveButton<T>(ctx.e, ctx.buttons, ctx.converter);

			ImGui::PopStyleVar();

			if (removed)
			{
				return {};
			}

			if (!open)
			{
				return {};
			}

			if (!BeginComponentTable())
			{
				return {};
			}

			const auto report = DrawComponent<T>(ctx.e, ctx.primaryRepo, ctx.spritePicker);

			if ((report.editState & PropertyEditState::Started) != 0)
			{
				const auto& cmp = ctx.e.GetComponent<T>();

				ComponentEditHistory::BeginComponentEdit(ctx.e, cmp);

				LOG_DEBUG_FMT("Began Component Edit : {} (Size: {})",
					GuiComponentName<T>::name, ComponentEditHistory::GetRecordsSize());
			}

			if (((report.editState & PropertyEditState::Active) == 0) &&
				((report.editState & PropertyEditState::Finished) != 0))
			{
				const auto& cmp = ctx.e.GetComponent<T>();

				ComponentEditHistory::EndComponentEdit(ctx.e, cmp);

				LOG_DEBUG_FMT("Ended Component Edit : {} (Size: {})",
					GuiComponentName<T>::name, ComponentEditHistory::GetRecordsSize());
			}

			EndComponentTable();

			return report;
		};

		assert(ctx.e.HasComponent<Name>());

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 6));

		ImGui::Text(ctx.e.GetComponent<Name>().value.c_str());

		ImGui::PopStyleVar();

		HandleUndoRedoButtons(ctx.converter, ctx.buttons);

		DrawComponentReport report{};

		report |= ((draw.template operator()<Ts>(ctx)), ...);

		return report;
	}
};

DrawComponentReport DrawComponents(DrawComponentsContext& ctx)
{
	return draw_components<EditorComponentPanel::GuiEditableComponentTypeList>::call(ctx);
}

/** @} */

} // unnamed

//void EditorComponentPanel::Update(Entity e, SceneFixture& fixture)
//{
//	if (!e.IsValid())
//	{
//		LOG_ERROR("Entity was invalid");
//		Reset();
//
//		return;
//	}
//
//	auto& auxRepo = fixture.GetAuxTextureRepository();
//	assert(auxRepo);
//
//	auto ctx = DrawComponentsContext{
//		.e = e,
//		.buttons = buttons_,
//		.componentHeaderOpen = componentHeaderOpen_,
//		.primaryRepo = fixture.GetTextureRepository(),
//		.spritePicker = spritePicker_,
//		.converter = GuiTextureConverter{*auxRepo}
//	};
//
//	ImGui::BeginChild("Components", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);
//
//	const auto report = DrawComponents(ctx);
//	if (!componentBuilders_.HasActiveBuilder() && 
//		report.openBuilder != kInvalidComponentBuilderType)
//	{
//		componentBuilders_.SetActiveBuilder(report.openBuilder);
//	}
//
//	ImGui::Separator();
//
//	DrawAddComponentOptions(e, componentHeaderOpen_);
//
//	ImGui::EndChild();
//
//	if (componentBuilders_.HasActiveBuilder())
//	{
//		ImGui::BeginChild("ComponentBuilder", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);
//
//		const bool built = componentBuilders_.DrawActiveBuilder(e, fixture);
//		if (built)
//		{
//			componentBuilders_.ClearActiveBuilder();
//		}
//
//		ImGui::EndChild();
//	}
//}
//
//Result<Void> EditorComponentPanel::LoadResources(SceneFixture& fixture)
//{
//	TRY(ResourcePath::Sprite("ui/editor/delete_icon.png"), deleteIconPath);
//	TRY(ResourcePath::Sprite("ui/editor/visibility_on_icon.png"), visibleOnIconPath);
//	TRY(ResourcePath::Sprite("ui/editor/visibility_off_icon.png"), visibleOffIconPath);
//	TRY(ResourcePath::Sprite("ui/editor/undo_icon.png"), undoIconPath);
//	TRY(ResourcePath::Sprite("ui/editor/redo_icon.png"), redoIconPath);
//
//	auto& auxRepo = fixture.GetAuxTextureRepository();
//	if (!auxRepo)
//	{
//		return MAKE_ERROR("Aux TextureRepository was null");
//	}
//	auto& spriteAtlas = auxRepo->GetSpriteAtlas();
//
//	TRY_ASSIGN(buttons_.remove.defaultSprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(deleteIconPath) }));
//	TRY_ASSIGN(buttons_.hide.defaultSprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(visibleOnIconPath) }));
//	TRY_ASSIGN(buttons_.hide.activatedSprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(visibleOffIconPath) }));
//	TRY_ASSIGN(buttons_.undo.sprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(undoIconPath) }));
//	TRY_ASSIGN(buttons_.redo.sprite, spriteAtlas.LoadSprite(
//		fixture.GetRenderer(), { .filepath = std::move(redoIconPath) }));
//
//	return kVoid;
//}
//
//Result<Void> EditorComponentPanel::Init(SceneFixture& fixture)
//{
//	TRY(LoadResources(fixture));
//
//	TRY(componentBuilders_.Init(fixture));
//
//	return kVoid;
//}
//
//void EditorComponentPanel::ClearState()
//{
//	componentHeaderOpen_.Reset();
//	componentBuilders_.ClearActiveBuilder();
//}

void EditorComponentPanel::UpdateImpl(Entity e, SceneFixture& fixture)
{
	if (!e.IsValid())
	{
		LOG_ERROR("Entity was invalid");
		ClearState();

		return;
	}

	auto& auxRepo = fixture.GetAuxTextureRepository();
	assert(auxRepo);

	auto ctx = DrawComponentsContext{
		.e = e,
		.buttons = buttons_,
		.componentHeaderOpen = componentHeaderOpen_,
		.primaryRepo = fixture.GetTextureRepository(),
		.spritePicker = spritePicker_,
		.converter = GuiTextureConverter{*auxRepo}
	};

	ImGui::BeginChild("Components", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

	const auto report = DrawComponents(ctx);
	if (!componentBuilders_.HasActiveBuilder() &&
		report.openBuilder != kInvalidComponentBuilderType)
	{
		componentBuilders_.SetActiveBuilder(report.openBuilder);
	}

	ImGui::Separator();

	DrawAddComponentOptions(e, componentHeaderOpen_);

	ImGui::EndChild();

	if (componentBuilders_.HasActiveBuilder())
	{
		ImGui::BeginChild("ComponentBuilder", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

		const bool built = componentBuilders_.DrawActiveBuilder(e, fixture);
		if (built)
		{
			componentBuilders_.ClearActiveBuilder();
		}

		ImGui::EndChild();
	}
}

Result<Void> EditorComponentPanel::LoadResources(SceneFixture& fixture)
{
	TRY(ResourcePath::Sprite("ui/editor/delete_icon.png"), deleteIconPath);
	TRY(ResourcePath::Sprite("ui/editor/visibility_on_icon.png"), visibleOnIconPath);
	TRY(ResourcePath::Sprite("ui/editor/visibility_off_icon.png"), visibleOffIconPath);
	TRY(ResourcePath::Sprite("ui/editor/undo_icon.png"), undoIconPath);
	TRY(ResourcePath::Sprite("ui/editor/redo_icon.png"), redoIconPath);

	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Aux TextureRepository was null");
	}
	auto& spriteAtlas = auxRepo->GetSpriteAtlas();

	TRY_ASSIGN(buttons_.remove.defaultSprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(deleteIconPath) }));
	TRY_ASSIGN(buttons_.hide.defaultSprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(visibleOnIconPath) }));
	TRY_ASSIGN(buttons_.hide.activatedSprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(visibleOffIconPath) }));
	TRY_ASSIGN(buttons_.undo.sprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(undoIconPath) }));
	TRY_ASSIGN(buttons_.redo.sprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(redoIconPath) }));

	return kVoid;
}

Result<Void> EditorComponentPanel::InitImpl(SceneFixture& fixture)
{
	TRY(LoadResources(fixture));

	TRY(componentBuilders_.Init(fixture));

	return kVoid;
}

void EditorComponentPanel::ClearStateImpl()
{
	componentHeaderOpen_.Reset();
	componentBuilders_.ClearActiveBuilder();
}

void EditorComponentPanel::TearDownImpl() {}


} // ui

#endif