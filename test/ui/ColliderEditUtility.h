#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"
#include "InspectorCommon.h"

class SceneFixture;

namespace ui {

class ColliderEditUtility
{
public:
	enum class EditMode
	{
		Unknown,
		NoActiveShape,
		NeutralCommitted,
		PlacingPoints,
		DraggingPoints,
		DraggingShape
	};

	struct State
	{
		enum DrawTempPointAs
		{
			None,
			Mouse,
			Connect
		};

		bool committed = false;
		DrawTempPointAs	drawTempPointAs = None;
		EditMode editMode = EditMode::NoActiveShape;
	};

	struct Point
	{
		struct RenderResources
		{
			GlyphTextWriter posWriter;
			Sprite circleIconSprite;
		};

		Entity textEntity;
		Entity circleIconEntity;

		bool IsValid() const;

		bool operator==(const Point&) const = default;

		bool SetPosition(SDL_FPoint newPos);
		SDL_FPoint GetPosition() const;
		void SetVisible(bool visible);

		void Destroy();

		static Point Create(SDL_FPoint pos, SDL_Color clr, const RenderResources& resources);
	};

	struct Shape
	{
		Entity drawEntity;
		std::vector<Point> pointEntities;

		bool IsValid() const;

		SDL_Rect GetBoundingBox() const;
		bool CanAddMorePoints() const;
		bool CanBuild() const;

		void Destroy();

		static Shape Create(B2Shape::Type type, SDL_Color clr);
	};

	struct ShapeData
	{
		static constexpr B2Shape::Type kUprightRect = static_cast<B2Shape::Type>(5);

		B2Shape::Type shapeType = B2Shape::Type::Invalid;
		std::vector<SDL_FPoint> points;
		std::optional<SDL_FPoint> tempPoint;
		SDL_Color color;
		size_t ordinal = 0;
		bool convertTempPointToScreen = false;
	};

	class DrawSystem
	{
	public:
		explicit DrawSystem(Camera& cam) : cam_(&cam) {}
		void Update(float);
	private:
		Camera* cam_ = nullptr;
	};

	static constexpr SDL_Color kTextColor = SDLite::kColorBlack;
	static constexpr int kFontSize = 36;
	static constexpr std::string_view kFontName = "GoNotoKurrent-Regular";
	static constexpr std::string_view kFontFileName = "GoNotoKurrent-Regular.ttf";
	static constexpr std::string_view kPointCircleIconSpriteName = "circle_icon";
	static constexpr std::string_view kPointCircleIconSpritePath = "shapes/circle_icon.png";
	static constexpr float kPointCircleColliderRadius = 8.0f;

	void Reset();

	void UpdateFocusedPoint(SDL_FPoint mousePos);

	static Entity MakeTextEntity(SDL_FPoint pos, const GlyphTextWriter& writer);
	static Entity MakePointCircleIconEntity(SDL_FPoint pos, const Sprite& sprite, SDL_Color clr);

	void Draw(B2ShapeParameters& shapeParams, SDL_FPoint bodyPos, const Camera& cam);

	bool CanBuild() const;

	Result<Void> Init(SceneFixture& fixture);

private:
	void AddPointToUprightRect(SDL_FPoint p, Shape& rectShape, ShapeData& rectData,
							   const Point::RenderResources& resources);
	void AddPointToPolygon(SDL_FPoint p, Shape& polyShape, ShapeData& polyData,
						   const Point::RenderResources& resources);
	bool AddPoint(SDL_FPoint p, const Point::RenderResources& resources);
	void TransformRectToPolygonShape(Shape& shape, ShapeData& data, const Point::RenderResources& resources);

	Point* GetFocusedPoint();
	const Point* GetFocusedPoint() const;
	bool SetFocusedPointPosition(SDL_FPoint pos);
	bool HasFocusedPoint() const;

	bool ShouldDrawPoint(const Point& point) const;
	void UpdatePointVisibility(Point& point);

	void ResolveTempPoint(ShapeData& data, SDL_FPoint mousePos);

	void UpdateShape(SDL_FPoint mousePos);
	bool MoveShape(SDL_FPoint amount);

	void UpdateState(const Camera& cam);

	void UpdateShapeParameters(B2ShapeParameters& shapeParams, SDL_FPoint bodyPos);
	B2ShapeParameters MakeShapeParameters(SDL_FPoint bodyPos) const;

	Result<Void> LoadResources(SceneFixture& fixture);

	Shape shape_;
	size_t focusedPointIndex_;
	State state_;
	Point::RenderResources pointRenderResources_;
};






} // ui

#endif