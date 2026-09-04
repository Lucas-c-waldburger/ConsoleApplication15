#include "ColliderEditUtility.h"

#if IMGUI_ENABLED
#include "../../systems/util/DebugDrawUtils.h"
#include "GuiMouse.h"
#include "../Fixtures.h"
#include "InspectorCommon.h"

namespace ui {

namespace {

constexpr std::array<SDL_FPoint, 4> MakeRectCorners(SDL_FPoint a, SDL_FPoint b)
{
	float minX = std::min(a.x, b.x);
	float maxX = std::max(a.x, b.x);
	float minY = std::min(a.y, b.y);
	float maxY = std::max(a.y, b.y);

	return {
		SDL_FPoint{minX, minY}, // top-left
		SDL_FPoint{maxX, minY}, // top-right
		SDL_FPoint{maxX, maxY}, // bottom-right
		SDL_FPoint{minX, maxY}  // bottom-left
	};
}

constexpr Dimensions<float> GetDimensionsFromRectPoints(const std::vector<SDL_FPoint>& pts)
{
	assert(pts.size() == 4);

	float minX = pts[0].x;
	float maxX = pts[0].x;
	float minY = pts[0].y;
	float maxY = pts[0].y;

	for (const auto& p : pts)
	{
		minX = std::min(minX, p.x);
		maxX = std::max(maxX, p.x);
		minY = std::min(minY, p.y);
		maxY = std::max(maxY, p.y);
	}

	return { maxX - minX, maxY - minY };
}

bool ContainsPoint(const std::vector<SDL_FPoint>& pts, SDL_FPoint p)
{
	for (const auto& pt : pts)
	{
		if (pt == p)
		{
			return true;
		}
	}
	return false;
}

bool IsUprightRect(const std::vector<SDL_FPoint>& pts)
{
	assert(pts.size() == 4);

	float xs[4], ys[4];

	for (int i = 0; i < 4; ++i)
	{
		xs[i] = pts[i].x;
		ys[i] = pts[i].y;
	}

	// find unique x values
	float x1 = xs[0], x2 = xs[0];
	for (int i = 1; i < 4; ++i)
	{
		if (!EqualsWithTolerance(xs[i], x1))
		{
			x2 = xs[i];
			break;
		}
	}

	// verify only 2 unique x values
	for (int i = 0; i < 4; ++i)
	{
		if (!EqualsWithTolerance(xs[i], x1) && !EqualsWithTolerance(xs[i], x2))
		{
			return false;
		}
	}

	// same for y
	float y1 = ys[0], y2 = ys[0];
	for (int i = 1; i < 4; ++i)
	{
		if (!EqualsWithTolerance(ys[i], y1))
		{
			y2 = ys[i];
			break;
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		if (!EqualsWithTolerance(ys[i], y1) && !EqualsWithTolerance(ys[i], y2))
		{
			return false;
		}
	}

	// now verify all 4 combinations exist
	return
		ContainsPoint(pts, { x1, y1 }) &&
		ContainsPoint(pts, { x2, y1 }) &&
		ContainsPoint(pts, { x2, y2 }) &&
		ContainsPoint(pts, { x1, y2 });
}

std::array<SDL_FPoint, 4>
MakePointsIntoUprightRect(const std::vector<SDL_FPoint>& pts)
{
	// --- helper: find best 2 clusters (1D k-means with k=2) ---
	static constexpr auto FindTwoCenters = [](const float v[4])
		-> std::pair<float, float> {
		float c1 = v[0];
		float c2 = v[3]; // start far apart

		for (int iter = 0; iter < 8; ++iter)
		{
			float sum1 = 0, sum2 = 0;
			int count1 = 0, count2 = 0;

			for (int i = 0; i < 4; ++i)
			{
				if (std::fabs(v[i] - c1) < std::fabs(v[i] - c2))
				{
					sum1 += v[i]; count1++;
				}
				else
				{
					sum2 += v[i]; count2++;
				}
			}

			if (count1 > 0) { c1 = sum1 / count1; }
			if (count2 > 0) { c2 = sum2 / count2; }
		}

		if (c1 > c2)
		{
			std::swap(c1, c2);
		}

		return { c1, c2 };
		};

	// --- gather axes ---
	float xs[4], ys[4];
	for (int i = 0; i < 4; ++i)
	{
		xs[i] = pts[i].x;
		ys[i] = pts[i].y;
	}

	auto [x1, x2] = FindTwoCenters(xs);
	auto [y1, y2] = FindTwoCenters(ys);

	// --- snap each point to nearest axis ---
	static constexpr auto snap = [](float v, float a, float b) {
		return (std::fabs(v - a) < std::fabs(v - b)) ? a : b;
	};

	std::array<SDL_FPoint, 4> out;

	for (int i = 0; i < 4; ++i)
	{
		out[i].x = snap(pts[i].x, x1, x2);
		out[i].y = snap(pts[i].y, y1, y2);
	}

	// --- enforce perfect rectangle ordering (optional but recommended) ---
	float minX = std::min({ out[0].x, out[1].x, out[2].x, out[3].x });
	float maxX = std::max({ out[0].x, out[1].x, out[2].x, out[3].x });
	float minY = std::min({ out[0].y, out[1].y, out[2].y, out[3].y });
	float maxY = std::max({ out[0].y, out[1].y, out[2].y, out[3].y });

	return {
		SDL_FPoint{minX, minY},
		SDL_FPoint{maxX, minY},
		SDL_FPoint{maxX, maxY},
		SDL_FPoint{minX, maxY}
	};
}

void FixUprightRect(ColliderEditUtility::Shape& rectShape, ColliderEditUtility::ShapeData& rectData)
{
	assert(rectData.shapeType == ColliderEditUtility::ShapeData::kUprightRect);
	assert(rectData.points.size() == 4);
	assert(rectShape.pointEntities.size() == 4);

	if (IsUprightRect(rectData.points))
	{
		return;
	}

	auto fixed = MakePointsIntoUprightRect(rectData.points);

	for (size_t i = 0; i < 4; ++i)
	{
		rectData.points[i] = fixed[i];
		rectShape.pointEntities[i].SetPosition(fixed[i]);
	}
}

SDL_FRect UprightRectPointsToSDLFRect(const std::vector<SDL_FPoint>& pts)
{
	if (!IsUprightRect(pts))
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	float minX = std::min({ pts[0].x, pts[1].x, pts[2].x, pts[3].x });
	float maxX = std::max({ pts[0].x, pts[1].x, pts[2].x, pts[3].x });
	float minY = std::min({ pts[0].y, pts[1].y, pts[2].y, pts[3].y });
	float maxY = std::max({ pts[0].y, pts[1].y, pts[2].y, pts[3].y });

	assert(minX < maxX);
	assert(minY < maxY);

	return {
		.x = minX,
		.y = minY,
		.w = maxX - minX,
		.h = maxY - minY
	};
}

void AdjustPoints(ColliderEditUtility::Shape& rectShape, ColliderEditUtility::ShapeData& rectData)
{
	if (rectData.shapeType == ColliderEditUtility::ShapeData::kUprightRect &&
		rectData.points.size() == 4)
	{
		assert(rectShape.pointEntities.size() == 4);
		FixUprightRect(rectShape, rectData);
	}
}

constexpr bool PointInCircle(const SDL_FPoint& p, const SDL_FPoint& center, float radius) noexcept
{
	float dx = p.x - center.x;
	float dy = p.y - center.y;

	return (dx * dx + dy * dy) <= (radius * radius);
}

constexpr bool PointInUprightRect(SDL_FPoint p, SDL_FPoint a, SDL_FPoint b)
{
	const float minX = std::min(a.x, b.x);
	const float maxX = std::max(a.x, b.x);
	const float minY = std::min(a.y, b.y);
	const float maxY = std::max(a.y, b.y);

	return p.x >= minX && p.x <= maxX && p.y >= minY && p.y <= maxY;
}

bool PointInPolygon(SDL_FPoint p, std::span<const SDL_FPoint> polygon)
{
	bool inside = false;

	const size_t n = polygon.size();

	for (size_t i = 0, j = n - 1; i < n; j = i++)
	{
		const auto a = polygon[i];
		const auto b = polygon[j];

		const bool crossesY = (a.y > p.y) != (b.y > p.y);
		if (crossesY)
		{
			const float intersectionX = a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y);
			if (p.x < intersectionX)
			{
				inside = !inside;
			}
		}
	}

	return inside;
}

bool CanBuildShapeImpl(const ColliderEditUtility::ShapeData& data)
{
	switch (data.shapeType)
	{
	case ColliderEditUtility::ShapeData::kUprightRect:
		return data.points.size() == 4;
	case B2Shape::Type::Polygon:
		return data.points.size() >= 3;
	default:
		return false;
	}
}

bool CanAddMorePointsImpl(const ColliderEditUtility::ShapeData& data)
{
	switch (data.shapeType)
	{
	case ColliderEditUtility::ShapeData::kUprightRect:
		return data.points.size() < 2;
	default:
		return data.points.size() < B2_MAX_POLYGON_VERTICES;
	}
}

bool CanCapShapePoints(const ColliderEditUtility::ShapeData& data)
{
	switch (data.shapeType)
	{
	case ColliderEditUtility::ShapeData::kUprightRect:
		return data.points.size() == 4;
	case B2Shape::Type::Polygon:
		return data.points.size() > 2;
	default:
		return false;
	}
}

} // unnamed

// POINT
bool ColliderEditUtility::Point::IsValid() const
{
	if (!textEntity.IsValid()) { return false; }
	if (!circleIconEntity.IsValid()) { return false; }

	if (!textEntity.HasComponent<Transform>()) { return false; }
	if (!circleIconEntity.HasComponent<Transform>()) { return false; }

	if (!textEntity.HasComponent<TextRenderableComponent>()) { return false; }
	if (!circleIconEntity.HasComponent<SpriteRenderableComponent>()) { return false; }

	return true;
}

bool ColliderEditUtility::Point::SetPosition(SDL_FPoint newPos)
{
	if (!(textEntity.HasComponent<Transform>() && circleIconEntity.HasComponent<Transform>()))
	{
		return false;
	}

	textEntity.GetComponent<Transform>().position = newPos;
	circleIconEntity.GetComponent<Transform>().position = newPos;

	return true;
}

SDL_FPoint ColliderEditUtility::Point::GetPosition() const
{
	if (!IsValid())
	{
		return { 0.0f, 0.0f };
	}

	return circleIconEntity.GetComponent<Transform>().position;
}


void ColliderEditUtility::Point::SetVisible(bool visible)
{
	textEntity.SetComponentVisibility<TextRenderableComponent>(visible);
	circleIconEntity.SetComponentVisibility<SpriteRenderableComponent>(visible);
}

void ColliderEditUtility::Point::Destroy()
{
	textEntity.Destroy();
	circleIconEntity.Destroy();
}

auto ColliderEditUtility::Point::Create(SDL_FPoint pos, SDL_Color clr, const RenderResources& resources) -> Point
{
	return {
		.textEntity = MakeTextEntity(pos, resources.posWriter),
		.circleIconEntity = MakePointCircleIconEntity(pos, resources.circleIconSprite, clr)
	};
}

// SHAPE
bool ColliderEditUtility::Shape::IsValid() const
{
	if (!drawEntity.IsValid() || !drawEntity.HasComponent<ColliderEditUtility::ShapeData>()) { return false; }
	if (!core::AllOf(pointEntities, [](const auto& p) { return p.IsValid(); })) { return false; }

	return true;
}

bool ColliderEditUtility::Shape::IsPointInside(SDL_FPoint p) const
{
	if (!drawEntity.HasComponent<ColliderEditUtility::ShapeData>())
	{
		return false;
	}

	const auto& data = drawEntity.GetComponent<ColliderEditUtility::ShapeData>();

	switch (data.shapeType)
	{
	case B2Shape::Type::Polygon:
		return PointInPolygon(p, data.points);
	case ShapeData::kUprightRect:
		return data.points.size() == 2 && PointInUprightRect(p, data.points[0], data.points[1]);
	default:
		return false;
	}
}

SDL_Rect ColliderEditUtility::Shape::GetBoundingBox() const
{
	if (!IsValid())
	{
		return { 0, 0, 0, 0 };
	}

	const auto& points = drawEntity.GetComponent<ColliderEditUtility::ShapeData>().points;

	return util::ComputeBoundingBox(points);
}

bool ColliderEditUtility::Shape::CanAddMorePoints() const
{
	if (drawEntity.HasComponent<ColliderEditUtility::ShapeData>())
	{
		return CanAddMorePointsImpl(drawEntity.GetComponent<ColliderEditUtility::ShapeData>());
	}

	return false;
}

bool ColliderEditUtility::Shape::CanBuild() const
{
	if (drawEntity.HasComponent<ColliderEditUtility::ShapeData>())
	{
		return CanBuildShapeImpl(drawEntity.GetComponent<ColliderEditUtility::ShapeData>());
	}

	return false;
}

void ColliderEditUtility::Shape::Destroy()
{
	drawEntity.Destroy();

	for (auto& pointE : pointEntities)
	{
		pointE.Destroy();
	}
	pointEntities.clear();
}

auto ColliderEditUtility::Shape::Create(B2Shape::Type type, SDL_Color clr) -> Shape
{
	Shape shape{};

	shape.drawEntity = ECS::CreateEntity();
	assert(shape.drawEntity.IsValid());

	shape.drawEntity.AddComponent<InspectorTag>();
	shape.drawEntity.AddComponent(ShapeData{
		.shapeType = type,
		.color = clr,
		.ordinal = 0
	});

	return shape;
}

// DRAWSYSTEM
void ColliderEditUtility::DrawSystem::Update(float)
{
	if (!cam_)
	{
		return;
	}

	auto entities = ECS::GetAllEntitiesWith<ColliderEditUtility::ShapeData>([](const auto& data) {
		return !data.points.empty();
	});

	std::ranges::sort(entities.begin(), entities.end(), [](const auto& e1, const auto& e2) {
		return e1.GetComponent<ColliderEditUtility::ShapeData>().ordinal >
			   e2.GetComponent<ColliderEditUtility::ShapeData>().ordinal;
		});

	auto& r = SDLite::Renderer();
	assert(r);

	SDL_Color origDrawColor = r.GetColor();
	SDL_Color currentDrawColor = origDrawColor;

	for (auto& e : entities)
	{
		auto& data = e.GetComponent<ShapeData>();

		if (data.color != currentDrawColor)
		{
			r.SetColor(data.color);
			currentDrawColor = data.color;
		}

		auto drawPoints = data.points | std::views::transform([this](const auto& p) {
			return cam_->WorldToScreen<SDL_FPoint>(p);
		}) | std::ranges::to<std::vector>();

		if (data.tempPoint.has_value())
		{
			drawPoints.emplace_back((data.convertTempPointToScreen 
				? cam_->WorldToScreen<SDL_FPoint>(*data.tempPoint) 
				: *data.tempPoint));
		}

		if (drawPoints.size() == 1)
		{
			SDL_RenderDrawPointF(r, drawPoints[0].x, drawPoints[0].y);
		}
		else
		{
			// treat upright rect as polygon if fully formed
			const auto shapeType = 
				(data.shapeType == ShapeData::kUprightRect && drawPoints.size() >= 4) 
					? B2Shape::Type::Polygon
					: data.shapeType;

			switch (shapeType)
			{
			case B2Shape::Type::Polygon:
			{
				SDL_RenderDrawLinesF(r, drawPoints.data(), drawPoints.size());

				break;
			}
			case ShapeData::kUprightRect:
			{
				assert(drawPoints.size() == 2);

				auto p1 = drawPoints[0];
				auto p2 = drawPoints[1];

				SDL_FRect rect{
					.x = std::min(p1.x, p2.x),
					.y = std::min(p1.y, p2.y),
					.w = std::fabs(p2.x - p1.x),
					.h = std::fabs(p2.y - p1.y)
				};

				SDL_RenderDrawRectF(r, &rect);

				break;
			}
			case B2Shape::Type::Circle:
			{
				SDL_RenderDrawPointsF(r, drawPoints.data(), drawPoints.size());
			}
			default:
			{
				break;
			}
			}
		}

		//if (data.tempPoint.has_value())
		//{
		//	data.points.pop_back();
		//}
	}

	if (currentDrawColor != origDrawColor)
	{
		r.SetColor(origDrawColor);
	}
}

// COLLIDEREDITUTILITY
void ColliderEditUtility::AddPointToUprightRect(SDL_FPoint p, Shape& rectShape, ShapeData& rectData,
												const Point::RenderResources& resources)
{
	if (rectData.points.empty())
	{
		// first point, just add the point entity and wait for second point to be added
		auto pointStruct = Point::Create(p, rectData.color, resources);
		assert(pointStruct.IsValid());

		rectData.points.emplace_back(p);
		rectShape.pointEntities.emplace_back(pointStruct);
	}
	else if (rectData.points.size() == 1)
	{
		// second point added, need to fill out all four points
		auto firstPoint = rectData.points[0];
		rectData.points.clear();

		const bool updateFocusPoint = GetFocusedPoint() == &rectShape.pointEntities[0] &&
									  focusedPointIndex_ != 0;

		auto firstPointStruct = rectShape.pointEntities[0];
		rectShape.pointEntities.clear();

		auto corners = MakeRectCorners(firstPoint, p);
		for (size_t i = 0; i < corners.size(); ++i)
		{
			auto corner = corners[i];

			Point newPointStruct{};

			if (updateFocusPoint && corner == firstPoint)
			{
				focusedPointIndex_ = i;
				newPointStruct = firstPointStruct;
				newPointStruct.SetPosition(corner); // just in case old pos was slightly different
			}
			else
			{
				newPointStruct = Point::Create(corner, rectData.color, resources);
				assert(newPointStruct.IsValid());
			}

			rectData.points.emplace_back(corner);
			rectShape.pointEntities.emplace_back(newPointStruct);
		}
	}
	else
	{
		LOG_ERROR("Unexpected number of points in upright rect shape data");
	}
}

void ColliderEditUtility::AddPointToPolygon(SDL_FPoint p, Shape& polyShape, ShapeData& polyData,
											const Point::RenderResources& resources)
{
	polyData.points.emplace_back(p);
	polyShape.pointEntities.emplace_back(Point::Create(p, polyData.color, resources));
}

bool ColliderEditUtility::AddPoint(SDL_FPoint p, const Point::RenderResources& resources)
{
	if (!(shape_.drawEntity.IsValid() && shape_.drawEntity.HasComponent<ShapeData>()))
	{
		return false;
	}

	auto& data = shape_.drawEntity.GetComponent<ShapeData>();
	if (!CanAddMorePointsImpl(data))
	{
		return false;
	}

	switch (data.shapeType)
	{
	case ShapeData::kUprightRect:
		AddPointToUprightRect(p, shape_, data, resources); break;
	case B2Shape::Type::Polygon:
		AddPointToPolygon(p, shape_, data, resources); break;
	default:
		return false;
	}

	return true;
}

void ColliderEditUtility::TransformRectToPolygonShape(Shape& shape, ShapeData& data, 
													  const Point::RenderResources& resources)
{
	assert(data.shapeType == ShapeData::kUprightRect);
	assert(data.points.size() == 2);
	assert(shape.pointEntities.size() == 2);

	auto corners = MakeRectCorners(data.points[0], data.points[1]);

	std::optional<SDL_FPoint> restorePointPos;
	auto cleanup = [&](size_t idx) {
		auto pos = data.points[idx];
		auto& pointStruct = shape.pointEntities[idx];

		if (GetFocusedPoint() == &pointStruct)
		{
			assert(!restorePointPos.has_value());
			restorePointPos = pos;
		}

		pointStruct.textEntity.Destroy();
		pointStruct.circleIconEntity.Destroy();
		};

	cleanup(0);
	cleanup(1);

	shape.pointEntities.clear();

	data.points = { corners.begin(), corners.end() };
	for (size_t i = 0; i < data.points.size(); ++i)
	{
		auto point = Point::Create(data.points[i], data.color, resources);
		assert(point.IsValid());

		shape.pointEntities.emplace_back(point);

		if (restorePointPos.has_value() &&
			EqualsWithTolerance(restorePointPos->x, data.points[i].x) &&
			EqualsWithTolerance(restorePointPos->y, data.points[i].y))
		{
			focusedPointIndex_ = i;
		}
	}

	data.shapeType = B2Shape::Type::Polygon;
}

void ColliderEditUtility::UpdateFocusedPoint(SDL_FPoint mousePos)
{
	if (GuiMouse::IsLeftClicked())
	{
		int i = 0;
	}

	if (state_.editMode == EditMode::DraggingPoints)
	{
		assert(focusedPointIndex_ < shape_.pointEntities.size());

		float smallestDist = std::numeric_limits<float>::max();
		size_t smallestIdx = 0;

		for (size_t i = 0; i < shape_.pointEntities.size(); ++i)
		{
			const auto& point = shape_.pointEntities[i];
			const auto pointPos = point.GetPosition();

			const float dx = mousePos.x - pointPos.x;
			const float dy = mousePos.y - pointPos.y;
			const float dist = dx * dx + dy * dy;

			if (dist < smallestDist)
			{
				smallestDist = dist;
				smallestIdx = i;
			}
		}

		assert(smallestIdx < shape_.pointEntities.size());

		focusedPointIndex_ = smallestIdx;

		return;
	}

	for (size_t pointIdx = 0; pointIdx < shape_.pointEntities.size(); ++pointIdx)
	{
		auto& pointStruct = shape_.pointEntities[pointIdx];
		auto& pointCircleIconEnt = pointStruct.circleIconEntity;
		const auto& circleEntTf = pointCircleIconEnt.GetComponent<Transform>();

		if (PointInCircle(mousePos, circleEntTf.position, kPointCircleColliderRadius / 1.5f))
		{
			focusedPointIndex_ = pointIdx;

			return;
		}
	}

	focusedPointIndex_ = std::numeric_limits<size_t>::max();
}

Entity ColliderEditUtility::MakeTextEntity(SDL_FPoint pos, const GlyphTextWriter& writer)
{
	auto e = ECS::CreateEntity();
	if (!e.IsValid())
	{
		return {};
	}

	e.AddComponent<InspectorTag>();
	e.AddComponent(Transform{ .position = pos });
	e.AddComponent(TextRenderableComponent{
		.writer = writer,
		.formatting = {
			.bounds = { 80, 60 },
			.align = TextAlign::Center,
			.scaleToBounds = false
		},
		.profile = {
			.drawOrder = 1000,
			.mods = {
				.color = RGB::FromSDLColor(kTextColor)
			},
			.offset = { 15.0f, -18.0f }
		}
	});

	return e;
}

Entity ColliderEditUtility::MakePointCircleIconEntity(SDL_FPoint pos, const Sprite& sprite, SDL_Color clr)
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	e.AddComponent<InspectorTag>();
	e.AddComponent(Transform{ .position = pos, .scale = { 0.75f, 0.75f } });
	e.AddComponent(SpriteRenderableComponent{
		.sprite = sprite,
		.profile = {
			.drawOrder = 999,
			.mods = {
				.color = RGB::FromSDLColor(clr)
			}
		}
	});

	return e;
}

auto ColliderEditUtility::GetFocusedPoint() -> Point*
{
	return (focusedPointIndex_ < shape_.pointEntities.size())
		? &shape_.pointEntities[focusedPointIndex_]
		: nullptr;
}

auto ColliderEditUtility::GetFocusedPoint() const -> const Point*
{
	return (focusedPointIndex_ < shape_.pointEntities.size())
		? &shape_.pointEntities[focusedPointIndex_]
		: nullptr;
}

bool ColliderEditUtility::SetFocusedPointPosition(SDL_FPoint pos)
{
	if (focusedPointIndex_ < shape_.pointEntities.size() && shape_.drawEntity.HasComponent<ShapeData>())
	{
		auto& data = shape_.drawEntity.GetComponent<ShapeData>();
		data.points[focusedPointIndex_] = pos;

		auto& pointStruct = shape_.pointEntities[focusedPointIndex_];	
		pointStruct.SetPosition(pos);

		return true;
	}

	return false;
}

bool ColliderEditUtility::HasFocusedPoint() const
{
	return focusedPointIndex_ < shape_.pointEntities.size() &&
		   shape_.pointEntities[focusedPointIndex_].IsValid();
}

bool ColliderEditUtility::ShouldDrawPoint(const Point& pointStruct) const
{
	return true;

	//if (state_.committed)
	//{
	//	if (state_.editMode == EditMode::DraggingShape)
	//	{
	//		return true;
	//	}

	//	if (GetFocusedPoint() == &pointStruct)
	//	{
	//		return true;
	//	}
	//}

	//return false;
}

void ColliderEditUtility::UpdatePointVisibility(Point& point)
{
	auto& pointTextEnt = point.textEntity;
	auto& pointCircleIconEnt = point.circleIconEntity;

	const bool displayPointEx = ShouldDrawPoint(point);
	if (displayPointEx)
	{
		assert(pointTextEnt.HasComponent<Transform>());
		const SDL_FPoint drawPos = pointTextEnt.GetComponent<Transform>().position;

		pointTextEnt.GetComponent<TextRenderableComponent>().writer.text =
			std::format("({}, {})", static_cast<int>(drawPos.x), static_cast<int>(drawPos.y));
	}

	assert(pointCircleIconEnt.HasComponent<Transform>());
	pointCircleIconEnt.GetComponent<Transform>().scale = (GetFocusedPoint() == &point)
		? SDL_FPoint{ 1.0f, 1.0f }
		: SDL_FPoint{ 0.75f, 0.75f };

	point.SetVisible(displayPointEx);
}

void ColliderEditUtility::ResolveTempPoint(ShapeData& data, SDL_FPoint mousePos)
{
	using enum State::DrawTempPointAs;

	data.tempPoint.reset();
	data.convertTempPointToScreen = false;

	switch (state_.drawTempPointAs)
	{
	case Mouse:
		data.tempPoint = mousePos;
		break;
	case Connect:
		if (CanCapShapePoints(data))
		{
			assert(!data.points.empty());
			data.tempPoint = data.points.front();
			data.convertTempPointToScreen = true;
		}
		break;
	case None: default:
		break;
	}
}

void ColliderEditUtility::UpdateShape(SDL_FPoint mousePos)
{
	assert(shape_.drawEntity.IsValid() && shape_.drawEntity.HasComponent<ShapeData>());

	auto& data = shape_.drawEntity.GetComponent<ShapeData>();
	data.color.a = 255;
	assert(data.points.size() == shape_.pointEntities.size());

	AdjustPoints(shape_, data);

	for (size_t i = 0; i < data.points.size(); ++i)
	{
		auto& pointStruct = shape_.pointEntities[i];

		assert(pointStruct.IsValid());

		auto& pointTextEnt = pointStruct.textEntity;
		auto& pointCircleIconEnt = pointStruct.circleIconEntity;

		const SDL_FPoint canonicalPointPos = pointCircleIconEnt.GetComponent<Transform>().position;

		data.points[i] = canonicalPointPos;
		pointTextEnt.GetComponent<Transform>().position = canonicalPointPos;

		UpdatePointVisibility(pointStruct);
	}

	ResolveTempPoint(data, mousePos);
}

bool ColliderEditUtility::MoveShape(SDL_FPoint amount)
{
	if (!shape_.IsValid())
	{
		return false;
	}

	auto& data = shape_.drawEntity.GetComponent<ShapeData>();

	assert(data.points.size() == shape_.pointEntities.size());

	for (size_t i = 0; i < data.points.size(); ++i)
	{
		data.points[i] += amount;
		shape_.pointEntities[i].textEntity.GetComponent<Transform>().position += amount;
		shape_.pointEntities[i].circleIconEntity.GetComponent<Transform>().position += amount;
	}

	return true;
}

void ColliderEditUtility::Reset()
{
	shape_.Destroy();
	focusedPointIndex_ = std::numeric_limits<size_t>::max();
	state_ = {};
}

void ColliderEditUtility::UpdateState(const Camera& cam)
{
	if (GuiMouse::InsideEditorWindow())
	{
		state_.drawTempPointAs = State::DrawTempPointAs::Connect;
		if (state_.committed)
		{
			state_.editMode = EditMode::NeutralCommitted;
		}

		return;
	}

	if (GuiMouse::IsRightClicked())
	{
		state_.committed = !state_.committed;
		state_.drawTempPointAs = state_.committed 
			? State::DrawTempPointAs::Connect 
			: State::DrawTempPointAs::Mouse;
		
		if (state_.committed)
		{
			state_.editMode = EditMode::NeutralCommitted;
		}
	}

	// we know mouse is in edit area

	switch (state_.editMode)
	{
	case EditMode::NeutralCommitted:

		state_.drawTempPointAs = State::DrawTempPointAs::Connect;

		if (GuiMouse::IsLeftClicked())
		{
			if (HasFocusedPoint())
			{
				state_.editMode = EditMode::DraggingPoints;

				return;
			}
			if (shape_.IsPointInside(cam.ScreenToWorld<SDL_FPoint>(GuiMouse::GetPosition())))
			{
				state_.editMode = EditMode::DraggingShape;

				return;
			}
		}

		return;
	case EditMode::PlacingPoints:
		if (state_.committed || !shape_.CanAddMorePoints())
		{
			state_.committed = true;
			state_.editMode = EditMode::NeutralCommitted;
			state_.drawTempPointAs = State::DrawTempPointAs::Connect;

			return;
		}
		if (GuiMouse::IsLeftClicked())
		{
			AddPoint(cam.ScreenToWorld<SDL_FPoint>(GuiMouse::GetPosition()), pointRenderResources_);

			state_.committed = state_.committed || !shape_.CanAddMorePoints();
		}

		state_.drawTempPointAs = state_.committed 
			? State::DrawTempPointAs::Connect 
			: State::DrawTempPointAs::Mouse;

		return;

	case EditMode::DraggingPoints:
		assert(state_.committed);

		state_.drawTempPointAs = State::DrawTempPointAs::Connect;

		if (GuiMouse::IsLeftReleased())
		{
			state_.editMode = EditMode::NeutralCommitted;

			return;
		}

		assert(HasFocusedPoint());
		SetFocusedPointPosition(cam.ScreenToWorld<SDL_FPoint>(GuiMouse::GetPosition()));

		return;

	case EditMode::DraggingShape:
		assert(state_.committed);

		state_.drawTempPointAs = State::DrawTempPointAs::Connect;

		if (GuiMouse::IsLeftReleased())
		{
			state_.editMode = EditMode::NeutralCommitted;
		}
		else
		{
			MoveShape(GuiMouse::GetRelativePosition());
		}

		return;
	}
}

void ColliderEditUtility::UpdateShapeParameters(B2ShapeParameters& shapeParams, SDL_FPoint bodyPos)
{
	if (!shape_.drawEntity.HasComponent<ShapeData>())
	{
		return;
	}

	const auto& data = shape_.drawEntity.GetComponent<ShapeData>();

	shapeParams.shapeType = (data.shapeType == ShapeData::kUprightRect)
		? B2Shape::Type::Polygon
		: data.shapeType;

	switch (data.shapeType)
	{
	case ShapeData::kUprightRect:
	{
		assert(data.points.size() == 2);

		SDL_FPoint center{
			data.points[0].x + (data.points[1].x - data.points[0].x) * 0.5f,
			data.points[0].y + (data.points[1].y - data.points[0].y) * 0.5f
		};

		shapeParams.dimensions = { 
			std::abs(data.points[1].x - data.points[0].x), 
			std::abs(data.points[1].y - data.points[0].y)
		};
		shapeParams.localPosition = center - bodyPos;

		return;
	}
	case B2Shape::Type::Polygon:
	{
		shapeParams.hull = data.points | std::views::transform([bodyPos](const auto& p) {
			return p - bodyPos;
		}) | std::ranges::to<std::vector>();

		break;
	}
	default:
		break;
	}
}

B2ShapeParameters ColliderEditUtility::MakeShapeParameters(SDL_FPoint bodyPos) const
{
	if (!shape_.drawEntity.HasComponent<ShapeData>())
	{
		return {};
	}

	const auto& data = shape_.drawEntity.GetComponent<ShapeData>();

	B2ShapeParameters shapeParams{};

	shapeParams.shapeType = (data.shapeType == ShapeData::kUprightRect)
		? B2Shape::Type::Polygon
		: data.shapeType;

	switch (data.shapeType)
	{
	case ShapeData::kUprightRect:
	{
		assert(data.points.size() == 2);

		SDL_FPoint center{
			data.points[0].x + (data.points[1].x - data.points[0].x) * 0.5f,
			data.points[0].y + (data.points[1].y - data.points[0].y) * 0.5f
		};

		shapeParams.dimensions = {
			std::abs(data.points[1].x - data.points[0].x),
			std::abs(data.points[1].y - data.points[0].y)
		};
		shapeParams.localPosition = center - bodyPos;

		break;
	}
	case B2Shape::Type::Polygon:
	{
		shapeParams.hull = data.points | std::views::transform([bodyPos](const auto& p) {
			return p - bodyPos;
			}) | std::ranges::to<std::vector>();

		break;
	}
	default:
		break;
	}

	return shapeParams;
}

void ColliderEditUtility::Draw(B2ShapeParameters& shapeParams, SDL_FPoint bodyPos, const Camera& cam)
{
	if (!shape_.IsValid())
	{
		shape_ = Shape::Create(B2Shape::Type::Polygon, SDLite::kColorOrange);
		state_.editMode = EditMode::PlacingPoints;
	}

	state_.drawTempPointAs = State::DrawTempPointAs::None;
	const auto mousePos = GuiMouse::GetPosition();

	UpdateFocusedPoint(cam.ScreenToWorld<SDL_FPoint>(mousePos));

	UpdateState(cam);

	UpdateShape(mousePos);

	shapeParams = MakeShapeParameters(bodyPos);
}

bool ColliderEditUtility::CanBuild() const
{
	return shape_.CanBuild();
}

Result<Void> ColliderEditUtility::LoadResources(SceneFixture& fixture)
{
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Auxilliary texture repository not initialized");
	}

	auto& fonts = auxRepo->GetFontAtlas();
	if (!fonts.HasFont(kFontName))
	{
		TRY(ResourcePath::Font(kFontFileName), fontPath);

		TRY(fonts.LoadFont(fixture.GetRenderer(), FontDescriptor{
			.fontName = std::string{kFontName},
			.filepath = std::move(fontPath),
			.fontSize = kFontSize
		}));
	}

	pointRenderResources_.posWriter = fonts.GetTextWriter(kFontName);
	if (!pointRenderResources_.posWriter.resourceHandle.IsValid())
	{
		return MAKE_ERROR_FMT("Could not retrieve text writer for font '{}'", kFontName);
	}

	auto& sprites = auxRepo->GetSpriteAtlas();
	if (!sprites.HasSprite(kPointCircleIconSpriteName))
	{
		TRY(ResourcePath::Sprite(kPointCircleIconSpritePath), pointIconPath);

		TRY(sprites.LoadSprite(fixture.GetRenderer(), {
			.spriteName = std::string{kPointCircleIconSpriteName},
			.filepath = std::move(pointIconPath)
		}));
	}

	pointRenderResources_.circleIconSprite = sprites.GetSprite(kPointCircleIconSpriteName);
	if (!pointRenderResources_.circleIconSprite.resourceHandle.IsValid())
	{
		return MAKE_ERROR_FMT("Could not retrieve sprite '{}'", kPointCircleIconSpriteName);
	}

	return kVoid;
}

Result<Void> ColliderEditUtility::Init(SceneFixture& fixture)
{
	if (!ECS::RegisterComponent<ShapeData>())
	{
		return MAKE_ERROR("Could not register ShapeData component");
	}
	
	fixture.RegisterSystem<DrawSystem>(Phase::Presentation, fixture.GetCamera());

	TRY(LoadResources(fixture));

	return kVoid;
}

} // ui

#endif