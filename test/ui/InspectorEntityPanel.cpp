#include "InspectorEntityPanel.h"
#include "GuiResource.h"
#include "GuiMouse.h"
#include "InspectorCommon.h"
#include <format>

#if IMGUI_ENABLED

namespace ui {

namespace {

static size_t entityNoNameCounter = 0;

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

size_t GetDrawOrder(const Entity& e)
{
	return e.HasComponent<SpriteRenderableComponent>()
		? e.GetComponent<SpriteRenderableComponent>().profile.drawOrder :
		e.HasComponent<TextRenderableComponent>()
		? e.GetComponent<TextRenderableComponent>().profile.drawOrder :
		std::numeric_limits<size_t>::max();
}

bool IsHoveredEntityChildOf(const Entity& e)
{
	return e.HasComponent<Children>([](const Children& chs) {
		return chs.childEntityIds.contains(InspectorEntityPanel::Selection().entityId);
	});
}

} // unnamed

void InspectorEntityPanel::SelectionBoxRenderer::Update(float)
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

void InspectorEntityPanel::AssignEntityName(Entity& e)
{
	static constexpr std::string_view kNoNameFmt = "Entity ({})";

	if (!e.HasComponent<Name>([](const auto& nm) { return !nm.value.empty(); }))
	{
		e.AddComponent<Name>().value = std::format(kNoNameFmt, entityNoNameCounter++);
	}
}

void InspectorEntityPanel::RemoveStaleEntity(Entity& e)
{
	auto it = selectionBoxes_.find(selection_.entityId);
	assert(it != selectionBoxes_.end());

	auto boxE = ECS::GetEntityByID(it->second);
	assert(boxE.IsValid());

	boxE.Destroy();

	selectionBoxes_.erase(selection_.entityId);
	selection_.Clear();

	buttons_.viewChildren.isHovered.erase(selection_.entityId);
}

bool InspectorEntityPanel::DrawAddEntityButton(const GuiTextureConverter& converter)
{
	auto& button = buttons_.addEntity;

	GuiTexture texture{ converter.FromSprite(button.sprite) };

	assert(texture.textureId != 0);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton("Add Entity", texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	button.isHovered = ImGui::IsItemHovered();

	bool added = false;
	if (pressed)
	{
		auto e = ECS::CreateEntity();
		assert(e.IsValid());

		auto [_, inserted] = selectionBoxes_.try_emplace(e.GetID(), MakeSelectionBox());
		assert(inserted);

		AssignEntityName(e);

		selection_.entityId = e.GetID();
		selection_.selectionType = SelectionType::Edit;
	}

	return added;
}

bool InspectorEntityPanel::DrawAddChildButton(Entity& e, const GuiTextureConverter& converter)
{
	auto& button = buttons_.addChild;

	GuiTexture texture{ converter.FromSprite(button.sprite) };

	assert(texture.textureId != 0);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	const auto tint = button.isHovered ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton("Add Child", texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	button.isHovered = ImGui::IsItemHovered();

	bool added = false;
	if (pressed)
	{
		auto rels = e.GetRelations();
		assert(!rels.IsChild());

		auto ch = rels.AddChild();

		auto [_, inserted] = selectionBoxes_.try_emplace(ch.GetID(), MakeSelectionBox());
		assert(inserted);

		AssignEntityName(ch);
	}

	return added;
}

bool InspectorEntityPanel::DrawViewChildrenButton(Entity& e, const GuiTextureConverter& converter)
{
	//auto& button = buttons_.viewChildren;
	auto& button = buttons_.addChild;

	//auto it = button.isHovered.find(e.GetID());
	//if (it == button.isHovered.end())
	//{
	//	return false;
	//}

	//auto& isHovered = it->second;

	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));

	const float buttonStartX = GetRightAlignButtonStartX(ImGui::GetFrameHeight(), 1);

	ImGui::SameLine(buttonStartX);

	GuiTexture texture{ converter.FromSprite(button.sprite) };
	auto h = ImGui::GetFrameHeight();
	texture.size.x = h;
	texture.size.y = h;

	assert(texture.textureId != 0);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	//const auto tint = isHovered ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);
	const auto tint = button.isHovered ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	//bool pressed = GuiImageButton("View Children", texture, ImVec4(0, 0, 0, 0), tint);
	const bool pressed = GuiImageButton("Add Child", texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	//ImGui::PopStyleVar();

	//isHovered = ImGui::IsItemHovered();
	button.isHovered = ImGui::IsItemHovered();

	//if (selection_.IsHovering() && IsHoveredEntityChildOf(e))
	//{
	//	// force draw children
	//	pressed = true;
	//}

	if (pressed)
	{
		auto rels = e.GetRelations();
		assert(!rels.IsChild());

		auto ch = rels.AddChild();

		auto [_, inserted] = selectionBoxes_.try_emplace(ch.GetID(), MakeSelectionBox());
		assert(inserted);

		AssignEntityName(ch);
	}

	bool changed = false;
	//if (pressed)
	//{
		ImGui::Indent();

		auto rels = e.GetRelations();
		if (rels.HasChildren())
		{
			auto chs = rels.GetChildren();
			for (auto& ch : chs)
			{
				ImGui::PushID(ch.GetID());

				bool chSelected = (ch.GetID() == selection_.entityId);

				AssignEntityName(ch);
				const auto& chName = ch.GetComponent<Name>().value;

				if (ImGui::Selectable(chName.c_str(), &chSelected,
					ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, ImGui::GetFrameHeight())))
				{
					selection_.entityId = ch.GetID();
					selection_.selectionType = SelectionType::Edit;
					changed = true;
				}
				else if (ImGui::IsItemHovered())
				{
					selection_.entityId = ch.GetID();
					selection_.selectionType = SelectionType::Hover;
					changed = true;
				}

				ImGui::PopID();
			}
		}

		//changed = DrawAddChildButton(e, converter);

		ImGui::Unindent();
	//}

	return changed;
}

void InspectorEntityPanel::DrawEntitySelections(ResourceContext& ctx)
{
	GuiTextureConverter converter{ ctx.textureRepo };

	auto es = ECS::GetAllEntitiesWith<Exclude<InspectorTag, Parent>>();

	for (auto& e : es)
	{
		ImGui::PushID(e.GetID());

		bool selected = (e.GetID() == selection_.entityId);

		AssignEntityName(e);
		const auto& name = e.GetComponent<Name>().value;

		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 6));

		const bool pressed = ImGui::Selectable(name.c_str(), &selected,
			ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, ImGui::GetFrameHeight()));

		//ImGui::PopStyleVar();

		if (pressed)
		{
			selection_.entityId = e.GetID();
			selection_.selectionType = SelectionType::Edit;
		}
		else if (ImGui::IsItemHovered())
		{
			selection_.entityId = e.GetID();
			selection_.selectionType = SelectionType::Hover;
		}

		DrawViewChildrenButton(e, converter);

		ImGui::PopID();
	}

	ImGui::Separator();

	DrawAddEntityButton(converter);
}

Entity_t InspectorEntityPanel::MakeSelectionBox()
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

void InspectorEntityPanel::UpdateSelectionBoxPositions(const Camera& cam)
{
	auto es = ECS::GetAllEntitiesWith<Exclude<InspectorTag>>();

	for (auto& e : es)
	{
		if (!selectionBoxes_.contains(e.GetID()))
		{
			auto [_, inserted] = selectionBoxes_.try_emplace(e.GetID(), MakeSelectionBox());
			assert(inserted);

			AssignEntityName(e);
		}
		if (!buttons_.viewChildren.isHovered.contains(e.GetID()))
		{
			auto [_, inserted] = buttons_.viewChildren.isHovered.try_emplace(
				e.GetID(), false);
			assert(inserted);
		}

		auto boxE = ECS::GetEntityByID(selectionBoxes_[e.GetID()]);
		assert(boxE.IsValid());

		assert(boxE.HasComponent<SelectionBox>());

		boxE.GetComponent<SelectionBox>().rect = GetScreenRectForEntity(e, cam);	
	}
}

void InspectorEntityPanel::UpdateSelectionBoxes()
{
	using Src = MouseInputSource;

	const auto mousePos = GuiMouse::GetPosition();
	const bool mouseInGuiWindow = GuiMouse::InsideEditorWindow();
	const bool mouseLeftClicked = GuiMouse::IsLeftClicked();
	const bool mouseWheelScrolled = GuiMouse::IsWheelScrolled();

	const bool noSelectionChange = selection_.IsEditing() && (!mouseLeftClicked || mouseInGuiWindow);
	if (noSelectionChange)
	{
		return;
	}

	auto es = ECS::GetAllEntitiesWith<Exclude<InspectorTag>>();
	std::vector<Entity_t> mouseOverEntities;

	for (auto& e : es)
	{
		assert(selectionBoxes_.contains(e.GetID()));

		auto boxE = ECS::GetEntityByID(selectionBoxes_[e.GetID()]);
		assert(boxE.IsValid());

		assert(boxE.HasComponent<SelectionBox>());
		boxE.SetComponentVisibility<SelectionBox>(false);

		auto& boxRect = boxE.GetComponent<SelectionBox>().rect;

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
				if (GuiMouse::GetScrollY() > 0.0f)
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

	if (selection_.IsHovering() || selection_.IsEditing())
	{
		auto it = selectionBoxes_.find(selection_.entityId);
		assert(it != selectionBoxes_.end());

		ECS::GetEntityByID(it->second).SetComponentVisibility<SelectionBox>(true);
	}
}

void InspectorEntityPanel::Update(ResourceContext& resourceCtx)
{
	DrawEntitySelections(resourceCtx);

	UpdateSelectionBoxes();

	if (selection_.IsEditing())
	{
		Entity e = ECS::GetEntityByID(selection_.entityId);
		if (!e.IsValid())
		{
			RemoveStaleEntity(e);
		}
	}
}

Result<Void> InspectorEntityPanel::Init(SceneFixture::SharedPtr& scene)
{
	TRY(ResourcePath::Sprite("ui/editor/add_entity_icon.png"), addEntityIconPath);
	TRY(ResourcePath::Sprite("ui/editor/add_child_icon.png"), addChildIconPath);
	TRY(ResourcePath::Sprite("ui/editor/view_children_icon.png"), viewChildrenIconPath);

	auto& spriteAtlas = scene->GetTextureRepository().GetSpriteAtlas();

	TRY_ASSIGN(buttons_.addEntity.sprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(addEntityIconPath) }));
	TRY_ASSIGN(buttons_.addChild.sprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(addChildIconPath) }));
	TRY_ASSIGN(buttons_.viewChildren.sprite, spriteAtlas.LoadSprite(
		scene->GetRenderer(), { .filepath = std::move(viewChildrenIconPath) }));

	bool registered = ECS::RegisterComponent<SelectionBox>();
	assert(registered);

	scene->RegisterSystem<SelectionBoxRenderer>(Phase::Presentation);

	return kVoid;
}

} // ui

#endif