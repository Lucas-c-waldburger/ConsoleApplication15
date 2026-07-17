#include "InspectorComponentPanel.h"

#if IMGUI_ENABLED
#include "GuiTexture.h"
#include "InspectorCommon.h"
#include "../../ecs/EntityPhysics.h"
#include "GuiMouse.h"
#include "ComponentEditHistory.h"

namespace ui {

namespace {

using UpdateReport = InspectorComponentPanel::UpdateReport;

static constexpr std::string_view kNoneName = "<none>";

template <typename TList>
struct draw_add_component_list;

template <typename...Ts>
struct draw_add_component_list<TypeList<Ts...>>
{
	static bool call(Entity& e)
	{
		static constexpr auto add = []<typename T>(Entity& e) {
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

				InspectorComponentPanel::GetComponentHeaderOpen().Set<T>(true);

				return true;
			}

			return false;
		};

		bool changed = false;
		((changed |= (add.template operator()<Ts>(e))), ...);

		return changed;
	}
};

bool DrawAddComponentOptions(Entity& e)
{
	bool added = false;

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		added = draw_add_component_list<
			InspectorComponentPanel::GuiAddableComponentTypeList>::call(e);

		if (added)
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return added;
}

template <typename T>
concept GuiRemovableComponent = 
	HasGuiComponentName<T> && public_mutable_component_v<T> && !std::same_as<T, Name>;

template <typename T>
concept GuiHideableComponent = HasGuiComponentName<T> && 
	public_mutable_component_v<T> && !std::same_as<T, Name>;

template <typename T>
bool DrawRemoveButton(Entity& e, const GuiTextureConverter& converter)
{
	auto& button = InspectorComponentPanel::GetButtons().remove;

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

template <typename T>
bool DrawHideButton(Entity& e, const GuiTextureConverter& converter)
{
	GuiTexture texture{};
	auto& button = InspectorComponentPanel::GetButtons().hide;

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

bool DrawUndoButton(const GuiTextureConverter& converter)
{
	return DrawUndoRedoButtonImpl(converter, InspectorComponentPanel::GetButtons().undo,
								  "UndoButton", !ComponentEditHistory::CanUndo());
}

bool DrawRedoButton(const GuiTextureConverter& converter)
{
	return DrawUndoRedoButtonImpl(converter, InspectorComponentPanel::GetButtons().redo,
								  "RedoButton", !ComponentEditHistory::CanRedo());
}

UpdateReport HandleUndoRedoButtons(const GuiTextureConverter& converter)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

	const float buttonStartX = GetRightAlignButtonStartX(ImGui::GetFrameHeight(), 2);

	ImGui::SameLine(buttonStartX);

	UpdateReport report = UpdateReport::None;

	if (DrawUndoButton(converter))
	{
		const int cursor = ComponentEditHistory::GetCursor();
		ComponentEditHistory::Undo();
		LOG_DEBUG_FMT("Undo : {} -> {} (Size: {})", cursor, 
			ComponentEditHistory::GetCursor(), ComponentEditHistory::GetRecordsSize());

		report = UpdateReport::HistoryCursorMoved;
	}

	ImGui::SameLine();

	if (DrawRedoButton(converter))
	{
		const int cursor = ComponentEditHistory::GetCursor();
		ComponentEditHistory::Redo();
		LOG_DEBUG_FMT("Redo : {} -> {} (Size: {})", cursor, 
			ComponentEditHistory::GetCursor(), ComponentEditHistory::GetRecordsSize());

		report = UpdateReport::HistoryCursorMoved;
	}

	ImGui::PopStyleVar();

	return report;
}

template <typename T>
struct draw_component : EntityFullAccessPrivelage
{
	static PropertyEditState call(InspectorComponentPanel::ResourceContext& ctx)
	{
		PushPropertyDepth();

		const auto state = Property("", ctx.entity.GetComponent<T>(GetEntityPassKey()));

		PopPropertyDepth();

		return state;
	}
};

template <>
struct draw_component<SpriteRenderableComponent>
{
	static PropertyEditState call(InspectorComponentPanel::ResourceContext& ctx)
	{
		assert(ctx.entity.HasComponent<SpriteRenderableComponent>());
		auto& r = ctx.entity.GetComponent<SpriteRenderableComponent>();

		auto state = PropertyGroup("sprite", [&] {
			auto st = PropertyEditState::None;

			auto nameOp = ctx.textureRepo.GetSpriteAtlas()
										 .GetSpriteInfo<&SpriteInfo::spriteName>(r.sprite);

			std::string name = nameOp.value_or(std::string{ kNoneName });

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
				auto& spritePicker = InspectorComponentPanel::GetSpritePicker();

				const bool spriteSelected = spritePicker.Draw(ctx.textureRepo);
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

		state |= PropertyGroup("profile", [&] { return Property("", r.profile); });

		return state;
	}
};

template <>
struct draw_component<RigidBody>
{
	static PropertyEditState call(InspectorComponentPanel::ResourceContext& ctx)
	{
		assert(ctx.entity.HasComponent<RigidBody>());
		auto& rb = ctx.entity.GetComponent<RigidBody>();

		auto state = PropertyEditState::None;

		PushPropertyDepth();

		if (rb.body.GetData().IsValid())
		{
			state |= Property("", ctx.entity.GetComponent<RigidBody>());
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button("Build"))
		{
			InspectorComponentPanel::GetActiveBuilderType() = ComponentBuilderType::RigidBody;
		}

		PopPropertyDepth();

		return state;
	}
};

template <>
struct draw_component<Collider>
{
	static PropertyEditState call(InspectorComponentPanel::ResourceContext& ctx)
	{
		assert(ctx.entity.HasComponent<Collider>());
		auto& col = ctx.entity.GetComponent<Collider>();

		auto state = PropertyEditState::None;

		PushPropertyDepth();

		if (col.shape.GetData().IsValid())
		{
			state |= Property("", ctx.entity.GetComponent<Collider>());
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::BeginDisabled(!ctx.entity.HasComponent<RigidBody>([](const auto& rb) {
			return rb.body.GetData().IsValid();
		}));

		if (ImGui::Button("Build"))
		{
			InspectorComponentPanel::GetActiveBuilderType() = ComponentBuilderType::Collider;
		}

		ImGui::EndDisabled();

		PopPropertyDepth();

		return state;
	}
};

template <typename T>
PropertyEditState DrawComponent(InspectorComponentPanel::ResourceContext& ctx)
{
	return draw_component<T>::call(ctx);
}

template <typename TList>
struct draw_components;

template <typename...Ts>
struct draw_components<TypeList<Ts...>> : EntityFullAccessPrivelage
{
	static void call(InspectorComponentPanel::ResourceContext& ctx, 
					 InspectorComponentPanel::UpdateReport& report)
	{
		static constexpr auto draw = []<typename T>
		(InspectorComponentPanel::ResourceContext& ctx, const GuiTextureConverter& converter)
		{
			if (!ctx.entity.HasComponent<T>())
			{
				return;
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 6));

			int headerFlags = ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DrawLinesFull;
			if (InspectorComponentPanel::GetComponentHeaderOpen().Test<T>())
			{
				headerFlags |= ImGuiTreeNodeFlags_DefaultOpen;
			}

			const bool open = ImGui::CollapsingHeader(GuiComponentName<T>::name.data(), headerFlags);

			ImGui::PopStyleVar();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

			const float buttonStartX = GetRightAlignButtonStartX(
				ImGui::GetFrameHeight(), 2
			);

			ImGui::SameLine(buttonStartX);

			DrawHideButton<T>(ctx.entity, converter);

			ImGui::SameLine();

			const bool removed = DrawRemoveButton<T>(ctx.entity, converter);

			ImGui::PopStyleVar();

			if (removed)
			{
				return;
			}

			if (!open)
			{
				return;
			}

			if (!BeginComponentTable())
			{
				return;
			}

			const auto state = DrawComponent<T>(ctx);

			if ((state & PropertyEditState::Started) != 0)
			{
				const auto& cmp = ctx.entity.GetComponent<T>();
				ComponentEditHistory::BeginComponentEdit(ctx.entity, cmp);
				LOG_DEBUG_FMT("Began Component Edit : {} (Size: {})", 
					GuiComponentName<T>::name, ComponentEditHistory::GetRecordsSize());
			}

			if (((state & PropertyEditState::Active) == 0) &&
				((state & PropertyEditState::Finished) != 0))
			{
				const auto& cmp = ctx.entity.GetComponent<T>();
				ComponentEditHistory::EndComponentEdit(ctx.entity, cmp);
				LOG_DEBUG_FMT("Ended Component Edit : {} (Size: {})", 
					GuiComponentName<T>::name, ComponentEditHistory::GetRecordsSize());

			}

			EndComponentTable();
		};

		GuiTextureConverter converter{ ctx.textureRepo };

		assert(ctx.entity.HasComponent<Name>());

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 6));

		ImGui::Text(ctx.entity.GetComponent<Name>().value.c_str());

		ImGui::PopStyleVar();

		report |= HandleUndoRedoButtons(converter);

		((draw.template operator()<Ts>(ctx, converter)), ...);
	}
};

void DrawComponents(InspectorComponentPanel::ResourceContext& ctx, UpdateReport& report)
{
	return draw_components<
		InspectorComponentPanel::GuiEditableComponentTypeList>::call(ctx, report);
}

} // unnamed

Result<Void> InspectorComponentPanel::Init(SceneFixture::SharedPtr& scene)
{
	TRY(ResourcePath::Sprite("ui/editor/delete_icon.png"), deleteIconPath);
	TRY(ResourcePath::Sprite("ui/editor/visibility_on_icon.png"), visibleOnIconPath);
	TRY(ResourcePath::Sprite("ui/editor/visibility_off_icon.png"), visibleOffIconPath);
	TRY(ResourcePath::Sprite("ui/editor/undo_icon.png"), undoIconPath);
	TRY(ResourcePath::Sprite("ui/editor/redo_icon.png"), redoIconPath);

	auto& spriteAtlas = scene->GetTextureRepository().GetSpriteAtlas();

	TRY_ASSIGN(buttons_.remove.defaultSprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(deleteIconPath) }));
	TRY_ASSIGN(buttons_.hide.defaultSprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(visibleOnIconPath) }));
	TRY_ASSIGN(buttons_.hide.activatedSprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(visibleOffIconPath) }));
	TRY_ASSIGN(buttons_.undo.sprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(undoIconPath) }));
	TRY_ASSIGN(buttons_.redo.sprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(redoIconPath) }));

	return kVoid;
}

InspectorComponentPanel::UpdateReport InspectorComponentPanel::Update(ResourceContext& ctx)
{
	UpdateReport report = UpdateReport::None;

	if (!ctx.entity.IsValid())
	{
		return report;
	}

	ImGui::BeginChild("Components", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

	DrawComponents(ctx, report);

	ImGui::Separator();

	DrawAddComponentOptions(ctx.entity);

	ImGui::EndChild();

	if (activeBuilderType_ != ComponentBuilderType::None)
	{
		ImGui::BeginChild("ComponentBuilder", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

		bool built = false;

		switch (activeBuilderType_)
		{
		case ComponentBuilderType::RigidBody:
		{
			built = GuiEditComponentBuilder<RigidBody>::Draw(ctx.entity, ctx.world);
			break;
		}
		case ComponentBuilderType::Collider:
		{
			if (!ctx.entity.HasComponent<RigidBody>([](const auto& rb) {
				return rb.body.GetData().IsValid();
			}))
			{
				activeBuilderType_ = ComponentBuilderType::None;
			}

			auto& body = ctx.entity.GetComponent<RigidBody>().body;

			built = GuiEditComponentBuilder<Collider>::Draw(ctx.entity, body);
			break;
		}
		default:
			break;
		}

		if (built)
		{
			activeBuilderType_ = ComponentBuilderType::None;
		}

		ImGui::EndChild();
	}

	return report;
}

void InspectorComponentPanel::ClearState()
{
	componentHeaderOpen_.Reset();
}

} // ui

#endif