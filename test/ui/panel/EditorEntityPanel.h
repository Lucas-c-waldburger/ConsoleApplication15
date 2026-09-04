#pragma once
#include "IEditorPanel.h"

#if IMGUI_ENABLED
#include "../../../ecs/Ecs.h"
#include "../../../camera/Camera.h"
#include "../InspectorCommon.h"
#include "../GuiTexture.h"

namespace ui {

class EditorEntityPanel : public BaseEditorPanel<EditorEntityPanel, PanelType::Entities>
{
public:
	friend class BaseEditorPanel<EditorEntityPanel, PanelType::Entities>;

	struct SelectionBox
	{
		SDL_FRect rect = { 0.0f, 0.0f, 0.0f, 0.0f };
		SDL_Color color = { 0, 0, 0, 255 };
	};

	struct SelectionBoxRenderer
	{
		void Update(float);
	};

	enum class SelectionType
	{
		None,
		Hover,
		Edit,
		ViewChildren
	};

	struct Selection
	{
		Entity_t entityId = kInvalidEntity;
		SelectionType selectionType = SelectionType::None;

		void Clear()
		{
			entityId = kInvalidEntity;
			selectionType = SelectionType::None;
		}
		bool IsHovering() const
		{
			return entityId != kInvalidEntity && selectionType == SelectionType::Hover;
		}
		bool IsEditing() const
		{
			return entityId != kInvalidEntity && selectionType == SelectionType::Edit;
		}
	};

	struct Buttons
	{
		SimpleButton addEntity;
		SimpleButton addChild;
		MapButton<Entity_t> viewChildren;

		void ClearHoverStates();
	};

	//EditorEntityPanel() : IEditorPanel(PanelType::Entities) {}
	//~EditorEntityPanel() override = default; 

	//void Update(Entity, SceneFixture& fixture) override;
	//Result<Void> Init(SceneFixture& fixture) override;
	//void ClearState() override;

	const Selection& GetSelection() { return selection_; }
	void ClearSelection();
	void UpdateSelectionBoxPositions(const Camera& cam);
	bool SetSelectedEntityForEdit(Entity_t id);

private:
	struct HoverStack
	{
		std::vector<Entity_t> entityIds;
		size_t currentIndex = 0;

		Entity_t GetCurrent() const
		{
			return currentIndex < entityIds.size() ? entityIds[currentIndex] : kInvalidEntity;
		}
		void operator++()
		{
			currentIndex = currentIndex + 1 >= entityIds.size() ? 0 : currentIndex + 1;
		}
		void operator--()
		{
			currentIndex = currentIndex <= 0 ? entityIds.size() - 1 : currentIndex - 1;
		}
		void Clear()
		{
			entityIds.clear();
			currentIndex = 0;
		}
	};

	void UpdateImpl(Entity, SceneFixture& fixture);
	Result<Void> InitImpl(SceneFixture& fixture);
	void ClearStateImpl();
	void TearDownImpl();

	void UpdateSelectionBoxes();
	void DrawEntitySelections(const Camera& camera, const TextureRepository& repo);
	bool DrawAddEntityButton(const GuiTextureConverter& converter);
	void DrawAddChildButton(Entity& e, const GuiTextureConverter& converter);
	void ClearSelectionBoxes();
	void RemoveStaleEntity(Entity& e);

	Result<Void> LoadResources(SceneFixture& fixture);

	static Entity_t MakeSelectionBox();
	static void AssignEntityName(Entity& e);

	std::unordered_map<Entity_t, Entity_t> selectionBoxes_;
	Selection selection_;
	HoverStack hoverStack_;
	Buttons buttons_;
};

} // ui

#endif