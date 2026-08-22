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

//bool InspectorEntityPanel::DrawAddChildButton(Entity& e, const GuiTextureConverter& converter)
//{
//	auto& button = buttons_.addChild;
//
//	GuiTexture texture{ converter.FromSprite(button.sprite) };
//
//	assert(texture.textureId != 0);
//
//	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
//	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
//	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
//
//	const auto tint = button.isHovered ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);
//
//	const bool pressed = GuiImageButton("Add Child", texture, ImVec4(0, 0, 0, 0), tint);
//
//	ImGui::PopStyleColor(3);
//
//	button.isHovered = ImGui::IsItemHovered();
//
//	bool added = false;
//	if (pressed)
//	{
//		auto rels = e.GetRelations();
//		assert(!rels.IsChild());
//
//		auto ch = rels.AddChild();
//
//		auto [_, inserted] = selectionBoxes_.try_emplace(ch.GetID(), MakeSelectionBox());
//		assert(inserted);
//
//		AssignEntityName(ch);
//	}
//
//	return added;
//}

void InspectorEntityPanel::DrawAddChildButton(Entity& e, const GuiTextureConverter& converter)
{
	auto& button = buttons_.addChild;

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

	const auto tint = button.isHovered ? ImVec4(1, 1, 1, 1) : ImVec4(.75f, .75f, .75f, 1);

	const bool pressed = GuiImageButton("Add Child", texture, ImVec4(0, 0, 0, 0), tint);

	ImGui::PopStyleColor(3);

	button.isHovered = ImGui::IsItemHovered();

	if (pressed)
	{
		auto rels = e.GetRelations();
		assert(!rels.IsChild());

		auto ch = rels.AddChild();

		auto [_, inserted] = selectionBoxes_.try_emplace(ch.GetID(), MakeSelectionBox());
		assert(inserted);

		AssignEntityName(ch);
	}

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
			}
			else if (ImGui::IsItemHovered())
			{
				selection_.entityId = ch.GetID();
				selection_.selectionType = SelectionType::Hover;
			}

			ImGui::PopID();
		}
	}

	ImGui::Unindent();
}

void InspectorEntityPanel::ClearSelectionBoxes()
{
	for (auto& [_, boxId] : selectionBoxes_)
	{
		if (auto e = ECS::GetEntityByID(boxId); e.IsValid())
		{
			e.Destroy();
		}
	}

	selectionBoxes_.clear();
}

void InspectorEntityPanel::ClearSelection()
{
	if (selection_.entityId == kInvalidEntity)
	{
		return;
	}

	auto it = selectionBoxes_.find(selection_.entityId);
	if (it != selectionBoxes_.end())
	{
		if (auto boxE = ECS::GetEntityByID(it->second); boxE.IsValid())
		{
			boxE.SetComponentVisibility<SelectionBox>(false);
		}
	}

	selection_.Clear();
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

		const bool pressed = ImGui::Selectable(name.c_str(), &selected,
			ImGuiSelectableFlags_AllowOverlap, ImVec2(0.0f, ImGui::GetFrameHeight()));

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

		DrawAddChildButton(e, converter);

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

bool InspectorEntityPanel::SetSelectedEntityForEdit(Entity_t id)
{
	if (selection_.entityId == id && selection_.selectionType == SelectionType::Edit)
	{
		return false;
	}

	auto newBoxIt = selectionBoxes_.find(id);
	if (newBoxIt == selectionBoxes_.end())
	{
		return false;
	}

	auto boxE = ECS::GetEntityByID(newBoxIt->second);
	if (!boxE.IsValid())
	{
		return false;
	}

	boxE.SetComponentVisibility<SelectionBox>(true);

	if (selection_.entityId != kInvalidEntity)
	{
		auto oldBoxIt = selectionBoxes_.find(selection_.entityId);
		if (oldBoxIt != selectionBoxes_.end())
		{
			auto oldE = ECS::GetEntityByID(oldBoxIt->second);
			if (oldE.IsValid())
			{
				oldE.SetComponentVisibility<SelectionBox>(false);
			}
		}
	}

	selection_.entityId = id;
	selection_.selectionType = SelectionType::Edit;

	return true;
}

void InspectorEntityPanel::UpdateSelectionBoxes()
{
	using Src = MouseInputSource;

	const auto mousePos = GuiMouse::GetPosition();
	const bool mouseInGuiWindow = GuiMouse::InsideEditorWindow();
	const bool mouseLeftClicked = GuiMouse::IsLeftClicked();
	const bool mouseRightClicked = GuiMouse::IsRightClicked();
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
			if (mouseRightClicked)
			{
				++hoverStack_;
			}
			//if (mouseWheelScrolled)
			//{
			//	if (GuiMouse::GetScrollY() > 0.0f)
			//	{
			//		++hoverStack_;
			//	}
			//	else
			//	{
			//		--hoverStack_;
			//	}

			//}

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
	Entity_t selectedEntityAtStart = selection_.IsEditing() ? selection_.entityId : kInvalidEntity;

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

Result<Void> InspectorEntityPanel::ResetForNewScene(SceneFixture& fixture)
{
	ClearSelectionBoxes();
	selection_.Clear();
	hoverStack_.Clear();
	buttons_.ClearHoverStates();

	//const auto& spriteAtlas = scene.GetTextureRepository().GetSpriteAtlas();
	//buttons_.addEntity.sprite = spriteAtlas.GetSprite("add_entity_icon");
	//buttons_.addChild.sprite = spriteAtlas.GetSprite("add_child_icon");
	//buttons_.viewChildren.sprite = spriteAtlas.GetSprite("view_children_icon");

	fixture.RegisterSystem<SelectionBoxRenderer>(Phase::Presentation);

	return kVoid;
}

Result<Void> InspectorEntityPanel::LoadResources(SceneFixture& fixture)
{
	TRY(ResourcePath::Sprite("ui/editor/add_entity_icon.png"), addEntityIconPath);
	TRY(ResourcePath::Sprite("ui/editor/add_child_icon.png"), addChildIconPath);
	TRY(ResourcePath::Sprite("ui/editor/view_children_icon.png"), viewChildrenIconPath);

	//auto& spriteAtlas = scene.GetTextureRepository().GetSpriteAtlas();
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Aux TextureRepository was null");
	}
	auto& spriteAtlas = auxRepo->GetSpriteAtlas();

	TRY_ASSIGN(buttons_.addEntity.sprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(addEntityIconPath) }));
	TRY_ASSIGN(buttons_.addChild.sprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(addChildIconPath) }));
	TRY_ASSIGN(buttons_.viewChildren.sprite, spriteAtlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(viewChildrenIconPath) }));

	return kVoid;
}

Result<Void> InspectorEntityPanel::Init(SceneFixture& fixture)
{
	TRY(LoadResources(fixture));

	bool registered = ECS::RegisterComponent<SelectionBox>();
	assert(registered);

	fixture.RegisterSystem<SelectionBoxRenderer>(Phase::Presentation);

	return kVoid;
}

void InspectorEntityPanel::Buttons::ClearHoverStates()
{
	addEntity.isHovered = false;
	addChild.isHovered = false;
	viewChildren.isHovered.clear();
}

} // ui

#endif