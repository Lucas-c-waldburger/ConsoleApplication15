#include "InspectorComponentPanel.h"

#if IMGUI_ENABLED
#include "GuiTexture.h"
#include "InspectorCommon.h"
#include "../../ecs/EntityPhysics.h"
#include "GuiMouse.h"

namespace ui {

namespace {

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
				e.AddComponent<T>();

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

template <typename T>
struct draw_component : EntityFullAccessPrivelage
{
	static bool call(InspectorComponentPanel::ResourceContext& ctx)
	{
		PushPropertyDepth();

		bool changed = Property("", ctx.entity.GetComponent<T>(GetEntityPassKey()));

		PopPropertyDepth();

		return changed;
	}
};

template <>
struct draw_component<SpriteRenderableComponent>
{
	static bool call(InspectorComponentPanel::ResourceContext& ctx)
	{
		assert(ctx.entity.HasComponent<SpriteRenderableComponent>());
		auto& r = ctx.entity.GetComponent<SpriteRenderableComponent>();

		bool changed = PropertyGroup("sprite", [&] {
			bool b = false;

			auto nameOp = ctx.textureRepo.GetSpriteAtlas()
										 .GetSpriteInfo<&SpriteInfo::spriteName>(r.sprite);

			std::string name = nameOp.value_or(std::string{ kNoneName });

			bool showPicker = Property("name", [&name] {
				ImGui::SameLine();
				bool b = ImGui::Button("Browse");
				ImGui::SameLine();
				GuiEditProperty(name);

				return b;
			});
			if (showPicker)
			{
				ImGui::OpenPopup("DrawPickerPopup");
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
					b = true;

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			return b;
		});

		changed |= PropertyGroup("profile", [&] { return Property("", r.profile); });

		return changed;
	}
};

template <>
struct draw_component<RigidBody>
{
	static bool call(InspectorComponentPanel::ResourceContext& ctx)
	{
		assert(ctx.entity.HasComponent<RigidBody>());
		auto& rb = ctx.entity.GetComponent<RigidBody>();

		bool changed = false;

		PushPropertyDepth();

		if (rb.body.GetData().IsValid())
		{
			changed |= Property("", ctx.entity.GetComponent<RigidBody>());
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button("Build"))
		{
			InspectorComponentPanel::GetActiveBuilderType() = ComponentBuilderType::RigidBody;
			changed = true;
		}

		PopPropertyDepth();

		return changed;
	}
};

template <>
struct draw_component<Collider>
{
	static bool call(InspectorComponentPanel::ResourceContext& ctx)
	{
		assert(ctx.entity.HasComponent<Collider>());
		auto& col = ctx.entity.GetComponent<Collider>();

		bool changed = false;

		PushPropertyDepth();

		if (col.shape.GetData().IsValid())
		{
			changed |= Property("", ctx.entity.GetComponent<Collider>());
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::BeginDisabled(!ctx.entity.HasComponent<RigidBody>([](const auto& rb) {
			return rb.body.GetData().IsValid();
		}));

		if (ImGui::Button("Build"))
		{
			InspectorComponentPanel::GetActiveBuilderType() = ComponentBuilderType::Collider;
			changed = true;
		}

		ImGui::EndDisabled();

		PopPropertyDepth();

		return changed;
	}
};

template <typename T>
bool DrawComponent(InspectorComponentPanel::ResourceContext& ctx)
{
	return draw_component<T>::call(ctx);
}

template <typename TList>
struct draw_components;

template <typename...Ts>
struct draw_components<TypeList<Ts...>> : EntityFullAccessPrivelage
{
	static bool call(InspectorComponentPanel::ResourceContext& ctx)
	{
		static constexpr auto draw = []<typename T>
		(InspectorComponentPanel::ResourceContext& ctx, const GuiTextureConverter& converter)
		{
			if (!ctx.entity.HasComponent<T>())
			{
				return false;
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
				return true;
			}

			if (!open)
			{
				return false;
			}

			//InspectorComponentPanel::GetComponentHeaderOpen().Set<T>(false);

			if (!BeginComponentTable())
			{
				return false;
			}

			const bool changed = DrawComponent<T>(ctx);

			EndComponentTable();

			return changed;
		};

		assert(ctx.entity.HasComponent<Name>());
		ImGui::Text(ctx.entity.GetComponent<Name>().value.c_str());

		GuiTextureConverter converter{ ctx.textureRepo };

		bool changed = false;
		((changed |= (draw.template operator()<Ts>(ctx, converter))), ...);

		return changed;
	}
};

bool DrawComponents(InspectorComponentPanel::ResourceContext& ctx)
{
	return draw_components<
		InspectorComponentPanel::GuiEditableComponentTypeList>::call(ctx);
}

} // unnamed

Result<Void> InspectorComponentPanel::Init(SceneFixture::SharedPtr& scene)
{
	TRY(ResourcePath::Sprite("ui/editor/delete_icon.png"), deleteIconPath);
	TRY(ResourcePath::Sprite("ui/editor/visibility_on_icon.png"), visibleOnIconPath);
	TRY(ResourcePath::Sprite("ui/editor/visibility_off_icon.png"), visibleOffIconPath);

	auto& spriteAtlas = scene->GetTextureRepository().GetSpriteAtlas();

	TRY_ASSIGN(buttons_.remove.defaultSprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(deleteIconPath) }));
	TRY_ASSIGN(buttons_.hide.defaultSprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(visibleOnIconPath) }));
	TRY_ASSIGN(buttons_.hide.activatedSprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(visibleOffIconPath) }));

	return kVoid;
}

void InspectorComponentPanel::Update(ResourceContext& ctx)
{
	if (!ctx.entity.IsValid())
	{
		return;
	}

	ImGui::BeginChild("Components", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

	DrawComponents(ctx);

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
}

void InspectorComponentPanel::ClearState()
{
	componentHeaderOpen_.Reset();
}

} // ui

#endif