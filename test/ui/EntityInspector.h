//#pragma once
//#include "../../FeatureFlags.h"
//
//#if IMGUI_ENABLED
//#include "../../ecs/Ecs.h"
//#include "../Fixtures.h"
//#include "../../camera/Camera.h"
//#include "gui_edit/GuiEditIncludes.h"
//#include "SpritePicker.h"
//
//namespace ui {
//
//class EntityInspector2
//{
//public:
//	struct SelectionBox
//	{
//		SDL_FRect rect = { 0.0f, 0.0f, 0.0f, 0.0f };
//		SDL_Color color = { 0, 0, 0, 255 };
//	};
//
//	struct SelectionBoxRenderer
//	{
//		void Update(float);
//	};
//
//	enum PanelType : uint8_t
//	{
//		ComponentList = 1 << 0,
//		RigidBodyBuilder = 1 << 1
//	};
//
//	struct PanelHeights
//	{
//		static constexpr float kSecondaryPanelSplitRatio = 0.7f;
//
//		float currentSplit = 1.0f;
//		float mainPanel = 0.0f;
//		float secondaryPanel = 0.0f;
//		float mainPanelContentHeight = 0.0f;
//		float secondaryPanelContentHeight = 0.0f;
//		float currentSecondaryPanelHeight = 0.0f;
//		std::optional<float> overrideMainPanelHeight;
//
//		void Update(bool drawSecondary);
//	};
//
//	struct PanelState
//	{
//		uint8_t panelType = 0;
//		float currentHeight = 0;
//	};
//
//	struct EditorState
//	{
//		uint8_t activePanels = 0;
//		ComponentBitset componentNodeExpandStates{ false };
//	};
//
//	struct ResourceContext
//	{
//		const Camera& camera;
//		TextureRepository& textureRepo;
//		B2World& world;
//
//		static ResourceContext Create(SceneFixture::SharedPtr& scene);
//	};
//
//	static Result<Void> Setup(SceneFixture::SharedPtr& scene);
//
//	static void Update(ResourceContext& resourceCtx);
//
//	void SetSelectedComponentBit(ComponentSignature bit) { selectedComponent_ = bit; }
//	static ComponentSignature GetSelectedComponentBit() { return selectedComponent_; }
//
//	static bool HandleSpriteComponentDraw(Entity& e, TextureRepository& repo);
//	static bool HandleSpriteComponent(SpriteRenderableComponent& r, TextureRepository& repo);
//
//	static inline SpritePicker spritePicker_;
//
//private:
//	enum class SelectionType
//	{
//		None,
//		Hover,
//		Edit
//	};
//
//	struct Selection
//	{
//		Entity_t entityId = kInvalidEntity;
//		SelectionType selectionType = SelectionType::None;
//
//		void Clear()
//		{
//			entityId = kInvalidEntity;
//			selectionType = SelectionType::None;
//		}
//		bool IsHovering() const 
//		{ 
//			return entityId != kInvalidEntity && selectionType == SelectionType::Hover;  
//		}
//		bool IsEditing() const
//		{
//			return entityId != kInvalidEntity && selectionType == SelectionType::Edit;
//		}
//	};
//
//	struct HoverStack
//	{
//		std::vector<Entity_t> entityIds;
//		size_t currentIndex = 0;
//
//		Entity_t GetCurrent() const
//		{
//			return currentIndex < entityIds.size() ? entityIds[currentIndex] : kInvalidEntity;
//		}
//
//		void operator++()
//		{
//			currentIndex = currentIndex + 1 >= entityIds.size()
//				? 0 : currentIndex + 1;
//		}
//		void operator--()
//		{
//			currentIndex = currentIndex <= 0
//				? entityIds.size() - 1 : currentIndex - 1;
//		}
//	};
//
//	EntityInspector2() = default;
//
//	static void UpdateSelectionBoxes(std::vector<Entity>& es, const Camera& cam);
//
//	static Entity_t MakeSelectionBox();
//
//	static SDL_FRect GetScreenRectForEntity(const Entity& e, const Camera& cam);
//
//	static void DrawEntitySelections(std::vector<Entity>& es);
//	static void DrawAddEntityOption();
//	static void DrawMainComponentEditor(ResourceContext& resourceCtx);
//
//	static void AssignEntityName(Entity& e);
//	static void RemoveStaleEntity(Entity& e);
//
//	static inline std::unordered_map<Entity_t, Entity_t> selectionBoxes_;
//	static inline Entity_t mouse_ = kInvalidEntity;
//	static inline Selection selection_;
//	static inline ComponentSignature selectedComponent_ = 0;
//	static inline HoverStack hoverStack_;
//	static inline size_t entityNoNameCounter_ = 0;
//	static inline EditorState editorState_{};
//	static inline PanelHeights panelHeights_{};
//};
//
//
//} // ui
//
//#endif