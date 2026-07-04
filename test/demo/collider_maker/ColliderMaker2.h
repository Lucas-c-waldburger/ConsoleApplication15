#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED 

#include <imgui.h>
#include "ColliderMakerCommon.h"
#include "../../ui/SpritePicker.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../deps/nlohmann/json.hpp"

namespace test {

class ColliderMaker2Context
{
public:
	class Mouse
	{
	public:
		static SDL_FPoint GetPos();
		static SDL_FPoint GetRelPos();
		static bool IsLeftClicked();
		static bool IsLeftReleased();
		static bool IsRightClicked();
		static bool IsRightReleased();
		static bool IsLeftHeld();

	private:
		Mouse() = default;
	};

	class MouseEntity
	{
	public:
		static constexpr float kMouseColliderRadius = 1.0f;

		MouseEntity() = default;

		static MouseEntity Create(EventBus& bus);

		bool IsValid() const { return entity_.IsValid(); }

		SDL_FPoint GetPosition() const;

	private:
		Entity entity_;
	};

	static inline SceneFixture::WeakPtr scene{};
	static inline GlyphTextWriter textWriter{};
	static inline Sprite pointCircleIconSprite{};
	static inline MouseEntity mouseEntity{};

private:
	ColliderMaker2Context() = default;
};

struct AppState;

class ShapeManager
{
public:
	struct Point
	{
		Entity textEntity;
		Entity circleIconEntity;

		bool IsValid() const;

		friend bool operator==(const Point& lhs, const Point& rhs)
		{
			return lhs.textEntity == rhs.textEntity &&
				   lhs.circleIconEntity == rhs.circleIconEntity;
		}

		bool SetPosition(SDL_FPoint newPos);

		SDL_FPoint GetPosition() const;

		void SetVisible(bool visible);

		static Point Create(SDL_FPoint pos, SDL_Color clr);
	};

	struct FocusedPoint
	{
		size_t shapeIndex = std::numeric_limits<size_t>::max();
		size_t pointIndex = std::numeric_limits<size_t>::max();

		bool IsValid() const
		{
			return shapeIndex != std::numeric_limits<size_t>::max() &&
				   pointIndex != std::numeric_limits<size_t>::max();
		}
	};

	struct Shape
	{
		std::string_view name;
		Entity drawEntity;
		std::vector<Point> pointEntities;

		bool IsValid() const;

		SDL_Rect GetBoundingBox() const;
	};

	struct ShapeState
	{
		bool isFocus = false;
		size_t oldOrdinal = std::numeric_limits<size_t>::max();
		//bool mouseInsideShape = false;

		constexpr bool NewFocusThisFrame() const noexcept
		{ 
			return oldOrdinal != std::numeric_limits<size_t>::max(); 
		}
	};

	static constexpr SDL_Color kTextColor = SDLite::kColorBlack;
	static constexpr std::string_view kPointCircleIconSpritePath = "shapes/circle_icon.png";
	static constexpr float kPointCircleColliderRadius = 8.0f;

	bool NewShape(std::string_view name, B2Shape::Type type, SDL_Color clr);
	bool AddPoint(std::string_view name, SDL_FPoint p);

	std::vector<std::string_view> GetShapeNames() const;

	void UpdateShapes(const AppState& appState);

	nlohmann::ordered_json SerializeShape(std::string_view name, SDL_FPoint origin) const;

	bool HasShape(std::string_view name) const;

	bool CanBuildShape(std::string_view name) const;

	bool CanAddMorePoints(std::string_view name) const;

	void UpdateFocusedPoint(const AppState& appState);

	bool HasFocusedPoint() const;

	bool SetFocusedPointPosition(SDL_FPoint newPos);

	void ClearFocusedPoint();

	Shape* GetShape(std::string_view name);
	const Shape* GetShape(std::string_view name) const;

	bool MoveShape(std::string_view name, SDL_FPoint amount);

private:
	static inline Point kInvalidPoint{};

	Point& GetFocusedPoint();

	static Entity MakeTextEntity(SDL_FPoint pos, const GlyphTextWriter& writer);
	static Entity MakePointCircleIconEntity(SDL_FPoint pos, const Sprite& sprite, SDL_Color clr);

	ShapeData* GetShapeData(std::string_view name);
	const ShapeData* GetShapeData(std::string_view name) const;

	void AddPointToUprightRect(SDL_FPoint p, Shape& rectShape, ShapeData& rectData);
	void AddPointToPolygon(SDL_FPoint p, Shape& polyShape, ShapeData& polyData);

	void UpdateShape(Shape& shape, const AppState& appState, const ShapeState& shapeState);

	void UpdatePointVisibility(Point& point, const AppState& appState, 
							   const ShapeState& shapeState);

	size_t GetOrdinalIfNewFocus(std::string_view name) const;

	std::optional<SDL_FPoint> ResolveTempPoint(const ShapeData& data, const AppState& appState, 
											   const ShapeState& shapeState) const;

	void TransformRectToPolygonShape(Shape& shape, ShapeData& data);

	bool ShouldDrawPoint(const Point& pointStruct, const AppState& appState, 
						 const ShapeState& shapeState);

	Dictionary<size_t> shapeIdxByName_;
	std::vector<Shape> shapes_;
	FocusedPoint focusedPoint_;
};

class SpriteManager
{
public:
	struct FocusedSprite
	{
		size_t spriteIdx = std::numeric_limits<size_t>::max();
	};

	bool NewSprite(std::string_view spriteName);

	std::vector<std::string_view> GetSpriteNames() const;


private:
	ui::SpritePicker spritePicker_;
	Dictionary<size_t> spriteIdxByName_;
	std::vector<Sprite> sprites_;
};


enum class EditMode
{
	Unknown,
	NoActiveShape,
	NeutralCommitted,
	SelectingSprite,
	PlacingSprite,
	PlacingPoints,
	DraggingSprite,
	DraggingPoints,
	DraggingShape
};

struct SelectedSprite
{
	static constexpr SDL_FPoint kDefaultSpriteScale = { 2.5f, 2.5f };

	static SelectedSprite Create();
	void Update(Sprite&& sprite, std::string&& spriteName);

	Entity entity;

	bool Empty() const;
	void Reset();
	SDL_FPoint GetPosition() const;
};

struct AppState
{
	enum DrawTempPointAs
	{
		None,
		Mouse,
		Connect
	};

	SelectedSprite activeSprite;
	std::string activeShapeName;
	SDL_FPoint mousePos = { 0.0f, 0.0f };
	bool committed = false;
	DrawTempPointAs	drawTempPointAs = None;
	EditMode editMode = EditMode::NoActiveShape;
};

class ColliderMaker2
{
public:
	static constexpr std::string_view kDefaultShapeName = "Shape";
	static constexpr std::string_view kJsonSaveFileName = "collider_maker_shapes.json";
	static constexpr std::string_view kFontFileName = "GoNotoKurrent-Regular.ttf";
	static constexpr int kFontSize = 36;

	static Result<Void> Init(SceneFixture::SharedPtr& fixture);

	static void Draw();

private:
	ColliderMaker2() = default;

	static bool HasActiveShape();

	static Result<Void> SaveShapesToFile(std::string_view fileName);

	static void UpdateAppState();

	static void DrawAvailableShapeButtons();
	static void DrawAddShapeButtons();

	static void HandleSpriteSelection();

	static inline ShapeManager shapeManager_{};
	static inline ui::SpritePicker spritePicker_{};
	static inline AppState appState_{};
};

} // test


#endif