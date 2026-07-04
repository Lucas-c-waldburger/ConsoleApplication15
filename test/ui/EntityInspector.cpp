#include "EntityInspector.h"
#include "GuiResource.h"
#include "gui_edit/GuiComponentNames.h"
#include "gui_edit/GuiEditPropertyTable.h"
#include "../../ecs/EntityEvents.h"
#include <array>
#include <iostream>

#if IMGUI_ENABLED

namespace ui {

namespace {

constexpr SDL_FRect kEmptyRect = { 0.0f, 0.0f, 0.0f, 0.0f };
constexpr Dimensions<float> kDefaultSelectBoxDims = { 70.0f, 70.0f };

constexpr std::array kSelectionBoxColors = {
	SDLite::kColorRed,
	SDLite::kColorBlue,
	SDLite::kColorGreen,
	SDLite::kColorPurple,
	SDLite::kColorOrange,
	SDLite::kColorPink,
	SDLite::kColorYellow
};

inline size_t currentColorIndex = 0;

template <typename T>
using GuiNamedPred = std::bool_constant<HasGuiComponentName<T>>;

using GuiNamedComponentTypeList = filter_types_t<CoreComponentTypeList, GuiNamedPred>;

template <typename T>
using GuiEditablePred = std::bool_constant<HasGuiEditProperty<T> && HasGuiComponentName<T>>;

using GuiEditableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiEditablePred>;

//using GuiBuilderComponentTypeList = TypeList<RigidBody, Collider>;

template <typename T>
concept RemovableComponent = (public_mutable_component_v<T> && !std::same_as<T, Name>);

template <typename T>
using GuiAddablePred = std::bool_constant<public_mutable_component_v<T> && HasGuiComponentName<T>>;

using GuiAddableComponentTypeList = filter_types_t<CoreComponentTypeList, GuiAddablePred>;

template <typename T>
bool DrawRemoveComponentButton(Entity& e, const EntityPassKey& k)
{
	bool changed = false;

	ImGui::PushID(index_of_v<T, ComponentTypeList>);

	if constexpr (!RemovableComponent<T>)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("x"))
	{
		e.RemoveComponent<T>(k);
		changed = true;
	}

	if constexpr (!RemovableComponent<T>)
	{
		ImGui::EndDisabled();
	}

	ImGui::PopID();

	return changed;
}

template <typename TList>
struct draw_components;

template <typename...Ts>
struct draw_components<TypeList<Ts...>> : EntityFullAccessPrivelage
{
	static bool call(Entity& entity, TextureRepository& repo)
	{
		static constexpr auto draw = []<typename T>(Entity& e, TextureRepository& repo, int& i) {
			bool changed = false;

			if (e.HasComponent<T>())
			{
				//if (DrawRemoveComponentButton<T>(e, GetEntityPassKey()))
				//{
				//	changed = true;
				//}
				//else
				//{
				//	ImGui::SameLine();

					changed = PropertyGroup(GuiComponentName<T>::name, [&e] {
						return Property("", e.GetComponent<T>(GetEntityPassKey()));
					});

					//if constexpr (std::same_as<T, SpriteRenderableComponent>)
					//{
					//	changed = EntityInspector2::HandleSpriteComponentDraw(e, repo);
					//}
					//else
					//{
						//changed = Property(GuiComponentName<T>::name,
						//	e.GetComponent<T>(GetEntityPassKey()));
					//}
				//}
			}

			return changed;
		};

		assert(entity.HasComponent<Name>());
		ImGui::Text(entity.GetComponent<Name>().value.c_str());

		bool changed = false;

		if (BeginPropertyTable())
		{
			int i = 0;
			((changed |= (draw.template operator() < Ts > (entity, repo, i))), ...);

			EndPropertyTable();
		}

		return changed;
	}
};

template <typename TList>
struct draw_add_component_list;

template <typename...Ts>
struct draw_add_component_list<TypeList<Ts...>>
{
	static bool call(Entity& e)
	{
		static constexpr auto add = []<typename T>(Entity & e) {

			if (ImGui::Selectable(GuiComponentName<T>::name.data()))
			{
				e.AddComponent<T>();

				ImGui::CloseCurrentPopup();

				return false;
			}

			return true;
		};

		return !(add.template operator()<Ts>(e) && ...);
	}
};

bool DrawAddComponentOptions(Entity& e)
{
	bool changed = false;

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("AddComponentPopup");
	}
	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		changed = draw_add_component_list<GuiAddableComponentTypeList>::call(e);

		ImGui::EndPopup();
	}

	return changed;
}

bool DrawComponents(Entity& e, TextureRepository& repo)
{
	return draw_components<GuiEditableComponentTypeList>::call(e, repo);
}

SDL_FRect ComputeBoundingBoxForGlyphCache(const std::vector<GlyphCacheData>& data)
{
	if (data.empty())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	float minX = data[0].destRect.x;
	float minY = data[0].destRect.y;
	float maxX = data[0].destRect.x + data[0].destRect.w;
	float maxY = data[0].destRect.y + data[0].destRect.h;

	for (size_t i = 1; i < data.size(); ++i)
	{
		minX = std::min(minX, data[i].destRect.x);
		minY = std::min(minY, data[i].destRect.y);
		maxX = std::max(maxX, data[i].destRect.x + data[i].destRect.w);
		maxY = std::max(maxY, data[i].destRect.y + data[i].destRect.h);
	}

	return { minX, minY, maxX - minX, maxY - minY };
}

size_t GetDrawOrder(const Entity& e)
{
	return e.HasComponent<SpriteRenderableComponent>()
		? e.GetComponent<SpriteRenderableComponent>().profile.drawOrder :
		e.HasComponent<TextRenderableComponent>()
		? e.GetComponent<TextRenderableComponent>().profile.drawOrder : 
		std::numeric_limits<size_t>::max();
}

} // unnamed


void EntityInspector2::SelectionBoxRenderer::Update(float)
{
	auto entities = ECS::GetAllEntitiesWith<SelectionBox>();
	if (entities.empty())
	{
		return;
	}

	const auto origColor = SDLite::Renderer().GetColor();

	for (auto& e : entities)
	{
		auto& box = e.GetComponent<SelectionBox>();

		SDLite::Renderer().SetColor(box.color);

		SDL_RenderFillRectF(SDLite::Renderer(), &box.rect);
	}

	SDLite::Renderer().SetColor(origColor);
}


void EntityInspector2::AssignEntityName(Entity& e)
{
	static constexpr std::string_view kNoNameFmt = "Entity ({})";

	if (!e.HasComponent<Name>([](const auto& nm) { return !nm.value.empty(); }))
	{
		e.AddComponent<Name>().value = std::format(kNoNameFmt, entityNoNameCounter_++);
	}
}

bool EntityInspector2::HandleSpriteComponentDraw(Entity& e, TextureRepository& repo)
{
	static const std::string kNone = "<none>";

	bool changed = false;

	if (ImGui::TreeNode("SpriteRenderableComponent"))
	{
		ImGui::Indent();

		assert(e.HasComponent<SpriteRenderableComponent>());

		auto& r = e.GetComponent<SpriteRenderableComponent>();
		auto nmOp = repo.GetSpriteAtlas().GetSpriteInfo<&SpriteInfo::spriteName>(r.sprite);

		if (nmOp.has_value())
		{
			GuiDraw(*nmOp, "sprite");
		}
		else
		{
			GuiDraw(kNone, "sprite");
		}

		ImGui::SameLine();
		if (ImGui::Button("Browse") || spritePicker_.IsOpen())
		{
			ImGui::OpenPopup("DrawPickerPopup");
		}
		if (ImGui::BeginPopup("DrawPickerPopup"))
		{
			const bool spriteSelected = spritePicker_.Draw(repo);
			if (spriteSelected)
			{ 
				auto spriteData = spritePicker_.GetSelectedSprite();
				assert(spriteData.has_value());
				assert(spriteData->sprite.resourceHandle.IsValid());

				r.sprite = std::move(spriteData->sprite);

				spritePicker_.Reset();
				changed = true;

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		changed |= GuiEdit(r.profile, "profile");

		ImGui::Unindent();
		ImGui::TreePop();
	}

	return changed;
}

void EntityInspector2::DrawAddEntityOption()
{
	if (ImGui::Button("Add Entity"))
	{
		auto e = ECS::CreateEntity();
		assert(e.IsValid());

		auto [_, inserted] = selectionBoxes_.try_emplace(e.GetID(), MakeSelectionBox());
		assert(inserted);

		AssignEntityName(e);

		EntityInspector2::selection_.entityId = e.GetID();
		EntityInspector2::selection_.selectionType = SelectionType::Edit;
	}
}


void EntityInspector2::DrawEntitySelections(std::vector<Entity>& es)
{
	for (auto& e : es)
	{
		ImGui::PushID(e.GetID());

		bool selected = (e.GetID() == EntityInspector2::selection_.entityId);

		AssignEntityName(e);
		const auto& name = e.GetComponent<Name>().value;

		if (ImGui::Selectable(name.c_str(), &selected))
		{
			EntityInspector2::selection_.entityId = e.GetID();
			EntityInspector2::selection_.selectionType = SelectionType::Edit;
		}
		else if (ImGui::IsItemHovered())
		{
			EntityInspector2::selection_.entityId = e.GetID();
			EntityInspector2::selection_.selectionType = SelectionType::Hover;
		}

		ImGui::PopID();
	}

	DrawAddEntityOption();
}


SDL_FRect EntityInspector2::GetScreenRectForEntity(const Entity& e, const Camera& cam)
{
	if (!e.HasComponent<Transform>())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	const auto& tf = e.GetComponent<Transform>();

	if (e.HasComponent<SpriteRenderableComponent>([](const auto& r) {
			return r.sprite.resourceHandle.IsValid();
		}))
	{
		const auto& rend = e.GetComponent<SpriteRenderableComponent>();
		auto plotRect = rend.sprite.plot.rect;

		float scaledW = static_cast<float>(plotRect.w) * tf.scale.x;
		float scaledH = static_cast<float>(plotRect.h) * tf.scale.y;

		SDL_FRect result{
			tf.position.x - (scaledW / 2.0f),
			tf.position.y - (scaledH / 2.0f),
			scaledW,
			scaledH
		};

		if (!rend.profile.isOverlay)
		{
			result = cam.WorldToScreen<SDL_FRect>(result);
		}

		return result;
	}

	if (e.HasComponent<TextRenderableComponent>())
	{
		if (e.HasComponent<TextRenderableGlyphCache>())
		{
			const auto& rend = e.GetComponent<TextRenderableComponent>();
			const auto& glyphs = e.GetComponent<TextRenderableGlyphCache>();

			auto result = ComputeBoundingBoxForGlyphCache(glyphs.cache);

			if (!rend.profile.isOverlay)
			{
				result = cam.WorldToScreen<SDL_FRect>(result);
			}

			return result;
		}
	}

	return {
		tf.position.x - (128.0f / 2.0f),
		tf.position.y - (128.0f / 2.0f),
		128.0f,
		128.0f
	};
}

Entity_t EntityInspector2::MakeSelectionBox()
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	const auto color = kSelectionBoxColors[currentColorIndex];
	currentColorIndex = currentColorIndex + 1 >= kSelectionBoxColors.size()
		? 0 : currentColorIndex + 1;

	e.AddComponent<InspectorTag>();
	auto& box = e.AddComponent(SelectionBox{ .color = color });
	box.color.a = 65;

	return e.GetID();
}

void EntityInspector2::UpdateSelectionBoxes(std::vector<Entity>& es, const Camera& cam)
{
	using Src = MouseInputSource;

	auto mouseE = ECS::GetEntityByID(EntityInspector2::mouse_);
	assert(mouseE.IsValid());
	assert(mouseE.HasComponent<MouseState>());
	auto& mouseState = mouseE.GetComponent<MouseState>();

	const auto mousePos = mouseState.values.cursor.absolutePos;
	const bool mouseInGuiWindow = ImGui::GetIO().WantCaptureMouse;
	const bool mouseLeftClicked = mouseState.inputs[Src::LeftButton].state == InputState::Pressed;
	const bool mouseRightClicked = mouseState.inputs[Src::RightButton].state == InputState::Pressed;
	const bool mouseWheelScrolled = mouseState.inputs[Src::Wheel].state == InputState::Pressed;

	const bool noSelectionChange = selection_.IsEditing() && (!mouseLeftClicked || mouseInGuiWindow);

	std::vector<Entity_t> mouseOverEntities;

	for (auto& e : es)
	{
		if (!selectionBoxes_.contains(e.GetID()))
		{
			auto [_, inserted] = selectionBoxes_.try_emplace(e.GetID(), MakeSelectionBox());
			assert(inserted);

			AssignEntityName(e);
		}

		auto boxE = ECS::GetEntityByID(selectionBoxes_[e.GetID()]);
		assert(boxE.IsValid());

		assert(boxE.HasComponent<SelectionBox>());

		auto& boxRect = boxE.GetComponent<SelectionBox>().rect = GetScreenRectForEntity(e, cam);
		if (boxRect == kEmptyRect)
		{
			continue;
		}

		if (noSelectionChange)
		{
			continue;
		}

		boxE.SetComponentVisibility<SelectionBox>(false);

		if (!mouseInGuiWindow)
		{
			if (!PointInsideRect(boxRect, mousePos))
			{
				continue;
			}

			mouseOverEntities.emplace_back(e.GetID());
		}
	}

	std::ranges::sort(mouseOverEntities, [](const auto& a, const auto& b) {
		return GetDrawOrder(ECS::GetEntityByID(a)) > GetDrawOrder(ECS::GetEntityByID(b));
	});

	if (mouseOverEntities != hoverStack_.entityIds)
	{
		hoverStack_.entityIds = std::move(mouseOverEntities);
		hoverStack_.currentIndex = 0;
	}

	if (!mouseInGuiWindow)
	{
		if (mouseLeftClicked)
		{
			if (selection_.IsEditing() || hoverStack_.entityIds.empty())
			{
				selection_.Clear();
			}
			else
			{
				assert(hoverStack_.currentIndex < hoverStack_.entityIds.size());

				selection_.entityId = hoverStack_.GetCurrent();
				selection_.selectionType = SelectionType::Edit;
			}
		}
		else if (!hoverStack_.entityIds.empty())
		{
			if (mouseWheelScrolled)
			{
				if (mouseState.values.wheel.scroll.y > 0.0f)
				{
					++hoverStack_;
				}
				else
				{
					--hoverStack_;
				}
				
			}

			selection_.entityId = hoverStack_.GetCurrent();
			selection_.selectionType = SelectionType::Hover;
		}
	}

	if (mouseRightClicked)
	{
		selection_.Clear();
	}

	if (selection_.IsHovering() || selection_.IsEditing())
	{
		auto it = selectionBoxes_.find(selection_.entityId);
		assert(it != selectionBoxes_.end());

		ECS::GetEntityByID(it->second).SetComponentVisibility<SelectionBox>(true);
	}
}

void EntityInspector2::Update(const Camera& cam, TextureRepository& repo)
{
	auto es = ECS::GetAllEntitiesWith<Exclude<InspectorTag>>();
	
	if (!selection_.IsEditing())
	{
		DrawEntitySelections(es);
	}

	UpdateSelectionBoxes(es, cam);

	if (selection_.IsEditing())
	{
		auto e = ECS::GetEntityByID(selection_.entityId);
		if (!e.IsValid())
		{
			auto it = selectionBoxes_.find(selection_.entityId);
			assert(it != selectionBoxes_.end());

			auto boxE = ECS::GetEntityByID(it->second);
			assert(boxE.IsValid());

			boxE.Destroy();

			selectionBoxes_.erase(selection_.entityId);
			selection_.Clear();
		}
		else
		{
			DrawComponents(e, repo);
			DrawAddComponentOptions(e);
		}
	}
}

void UpdateTest()
{
	Transform tf{
	.position = {2.56, -35.5},
	.rotation = 35.0f,
	.scale = { 1.5f, 2.0f }
	};
	SDL_Rect r{ 24, 634, 53, 66 };
	SDL_Color c{ 224, 128, 255, 255 };

	ImGui::BeginChild("InspectorBody", ImVec2(0, 0), false);

	if (BeginPropertyTable())
	{
		PropertyGroup("Transform", [&] {
			return Property("", tf);
		});
		//PropertyGroup("Transform", [&] {
		//	Property("position", tf.position);
		//	Property("rotation", tf.rotation);
		//	Property("scale", tf.scale);
		//	Property("rect", r);
		//	Property("color", c);

		//	//InnerPropertyGroup("color", [&] {
		//	//	//Property("", r);
		//	//	Property("", c);
		//	//});
		//});

		EndPropertyTable();
	}

	ImGui::EndChild();
}

void AssignGuiColors()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);

	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.125f, 0.125f, 0.135f, 1.00f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.145f, 0.145f, 0.155f, 1.00f);

	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);

	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.17f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.58f, 1.00f);

	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3f, 0.5f, 0.8f, 0.25f);

	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.65f, 0.70f, 1.00f);

	style.Colors[ImGuiCol_Header] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.19f, 0.19f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
}

void AssignGuiStyles()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.FrameRounding = 4;
	style.WindowRounding = 6;
	style.ScrollbarRounding = 4;
	style.GrabRounding = 4;

	//style.ItemSpacing = ImVec2(8.0f, 5.0f);
	//style.FramePadding = ImVec2(6.0f, 3.0f);
	//style.CellPadding = ImVec2(6.0f, 4.0f);

	AssignGuiColors();
}

Result<Void> EntityInspector2::Setup(SceneFixture::SharedPtr& scene)
{
	bool registered = ECS::RegisterComponent<InspectorTag>();
	assert(registered);
	registered = ECS::RegisterComponent<SelectionBox>();
	assert(registered);

	auto mouseE = ECS::CreateEntity();
	assert(mouseE.IsValid());
	mouseE.AddComponent<InspectorTag>();
	mouseE.AddComponent<MouseState>();

	EntityInspector2::mouse_ = mouseE.GetID();

	assert(scene->IsSystemRegistered<GuiSystem>());

	TRY(GuiResource::Init());

	AssignGuiStyles();

	//const bool success = scene->GetSystem<GuiSystem>().AddWidget(
	//	"Test", []{ UpdateTest(); });
	//assert(success);
	
	const auto& cam = scene->GetCamera();
	auto& repo = scene->GetTextureRepository();

	const bool success = scene->GetSystem<GuiSystem>().AddWidget(
		"Entity Inspector", [&cam, &repo]{ EntityInspector2::Update(cam, repo); });
	assert(success);

	scene->RegisterSystem<SelectionBoxRenderer>(Phase::Presentation);

	return kVoid;
}



} // ui

#endif