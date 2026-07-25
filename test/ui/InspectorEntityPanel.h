#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"
#include "../Fixtures.h"
#include "../../camera/Camera.h"
#include "InspectorCommon.h"
#include "GuiTexture.h"

namespace ui {

class InspectorEntityPanel
{
public:
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

	struct ResourceContext
	{
		const Camera& camera;
		const TextureRepository& textureRepo;
	};

	static Result<Void> Init(SceneFixture& scene);

	static void Update(ResourceContext& resourceCtx);

	static void UpdateSelectionBoxPositions(const Camera& cam);

	static const Selection& GetSelection() { return selection_; }

	static void ClearSelection() { selection_.Clear(); }
	 
	static bool SetSelectedEntityForEdit(Entity_t id);

	static Buttons& GetButtons() { return buttons_; }

	static Result<Void> ResetForNewScene(SceneFixture& scene);

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
			currentIndex = currentIndex + 1 >= entityIds.size()
				? 0 : currentIndex + 1;
		}
		void operator--()
		{
			currentIndex = currentIndex <= 0
				? entityIds.size() - 1 : currentIndex - 1;
		}

		void Clear()
		{
			entityIds.clear();
			currentIndex = 0;
		}
	};

	static Entity_t MakeSelectionBox();

	static void UpdateSelectionBoxes();

	static void DrawEntitySelections(ResourceContext& ctx);
	static bool DrawAddEntityButton(const GuiTextureConverter& converter);
	static void DrawAddChildButton(Entity& e, const GuiTextureConverter& converter);
	static void ClearSelectionBoxes();

	static void AssignEntityName(Entity& e);
	static void RemoveStaleEntity(Entity& e);

	static Result<Void> LoadResources(SceneFixture& scene);

	static inline std::unordered_map<Entity_t, Entity_t> selectionBoxes_;
	static inline Selection selection_;
	static inline HoverStack hoverStack_;
	static inline Buttons buttons_;
};

} // ui

#endif