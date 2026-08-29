#include "ColliderMaker2.h"
#include "ColliderDrawSystem.h"
#include <format>
#include "../../../serial/user_types/SDLJsonUserTypes.h"
#include "../../../systems/util/DebugDrawUtils.h"
#include "../../../ecs/EntityEvents.h"
#include "../../ui/TextureRepositoryContext.h"

#if IMGUI_ENABLED

namespace test {

using CTX = ColliderMaker2Context;

using enum MouseInputSource;

namespace {

std::string EditModeToString(EditMode mode)
{
	using enum EditMode;
	switch (mode)
	{
	case NoActiveShape: return "No Active Shape";
	case NeutralCommitted: return "Neutral Committed";
	case SelectingSprite: return "Selecting Sprite";
	case PlacingSprite: return "Placing Sprite";
	case PlacingPoints: return "Placing Points";
	case DraggingSprite: return "Dragging Sprite";
	case DraggingPoints: return "Dragging Points";
	case DraggingShape: return "Dragging Shape";
	case Unknown: default: return "Unknown";
	}
}

std::string ShapeTypeToString(B2Shape::Type type)
{
	using enum B2Shape::Type;
	switch (type)
	{
	case Circle: return "Circle";
	case Capsule: return "Capsule";
	case Segment: return "Segment";
	case Polygon: return "Polygon";
	case ChainSegment: return "ChainSegment";
	case ShapeData::kUprightRect: return "UprightRect";
	case Invalid: default: return "Invalid";
	}
}

std::array<SDL_FPoint, 4> MakeRectCorners(SDL_FPoint a, SDL_FPoint b)
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

//std::array<SDL_FPoint, 4> MakePointsIntoUprightRect(const std::vector<SDL_FPoint>& pts)
//{
//	float minX = pts[0].x;
//	float maxX = pts[0].x;
//	float minY = pts[0].y;
//	float maxY = pts[0].y;
//
//	for (const auto& p : pts)
//	{
//		minX = std::min(minX, p.x);
//		maxX = std::max(maxX, p.x);
//		minY = std::min(minY, p.y);
//		maxY = std::max(maxY, p.y);
//	}
//
//	return {
//		SDL_FPoint{minX, minY},
//		SDL_FPoint{maxX, minY},
//		SDL_FPoint{maxX, maxY},
//		SDL_FPoint{minX, maxY}
//	};
//}

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

void FixUprightRect(ShapeManager::Shape& rectShape, ShapeData& rectData)
{
	assert(rectData.shapeType == ShapeData::kUprightRect);
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

void AdjustPoints(ShapeManager::Shape& rectShape, ShapeData& rectData)
{
	if (rectData.shapeType == ShapeData::kUprightRect &&
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

MouseState GetMouse()
{
	auto scene = CTX::scene.lock();
	if (!scene || !scene->IsSystemRegistered<SDLInputSystem>())
	{
		return {};
	}

	const auto& inpSys = scene->GetSystem<SDLInputSystem>();

	return inpSys.GetMouseEventHandler().GetMouseState();
}

bool IsMouseImpl(MouseInputSource src, InputState st)
{
	return GetMouse().inputs[src].state == st;
}

bool CanBuildShapeImpl(const ShapeData& data)
{
	switch (data.shapeType)
	{
	case ShapeData::kUprightRect:
		return data.points.size() == 4;
	case B2Shape::Type::Polygon:
		return data.points.size() >= 3;
	default:
		return false;
	}
}

bool CanAddMorePointsImpl(const ShapeData& data)
{
	switch (data.shapeType)
	{
	case ShapeData::kUprightRect:
		return data.points.size() < 2;
	default:
		return data.points.size() < B2_MAX_POLYGON_VERTICES;
	}
}

bool CanCapShapePoints(const ShapeData& data)
{
	switch (data.shapeType)
	{
	case ShapeData::kUprightRect:
		return data.points.size() == 4;
	case B2Shape::Type::Polygon:
		return data.points.size() > 2;
	default:
		return false;
	}
}

} // unnamed

// MOUSE
SDL_FPoint CTX::Mouse::GetPos()
{
	return GetMouse().values.cursor.absolutePos;
}

SDL_FPoint CTX::Mouse::GetRelPos()
{
	return GetMouse().values.cursor.relativePos;
}

bool CTX::Mouse::IsLeftClicked()
{
	return IsMouseImpl(LeftButton, InputState::Pressed);
}

bool CTX::Mouse::IsRightClicked()
{
	return IsMouseImpl(RightButton, InputState::Pressed);
}

bool CTX::Mouse::IsLeftReleased()
{
	return IsMouseImpl(LeftButton, InputState::Released);
}

bool CTX::Mouse::IsLeftHeld()
{
	return IsMouseImpl(LeftButton, InputState::Held);
}

CTX::MouseEntity CTX::MouseEntity::Create(EventBus& bus)
{
	MouseEntity mouseEnt{};

	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	auto evs = e.GetEvents(bus);

	auto evResult = evs.OnInput(Cursor, [](const events::MouseInput& ev, Transform& tf) {
		tf.position = ev.values.cursor.absolutePos;
	});
	assert(evResult.Success());

	mouseEnt.entity_ = e;

	return mouseEnt;
}

SDL_FPoint CTX::MouseEntity::GetPosition() const
{
	if (!entity_.IsValid() || !entity_.HasComponent<Transform>())
	{
		return { 0.0f, 0.0f };
	}
	return entity_.GetComponent<Transform>().position;
}

// SHAPE MANAGER
bool ShapeManager::Point::IsValid() const
{
	if (!textEntity.IsValid()) { return false; }
	if (!circleIconEntity.IsValid()) { return false; }

	if (!textEntity.HasComponent<Transform>()) { return false; }
	if (!circleIconEntity.HasComponent<Transform>()) { return false; }

	if (!textEntity.HasComponent<TextRenderableComponent>()) { return false; }
	if (!circleIconEntity.HasComponent<SpriteRenderableComponent>()) { return false; }

	return true;
}

bool ShapeManager::Shape::IsValid() const
{
	if (name.empty()) { return false; }
	if (!drawEntity.IsValid() || !drawEntity.HasComponent<ShapeData>()) { return false; }
	if (!core::AllOf(pointEntities, [](const auto& p) { return p.IsValid(); })) { return false; }

	return true;
}

SDL_Rect ShapeManager::Shape::GetBoundingBox() const
{
	if (!IsValid())
	{
		return { 0, 0, 0, 0 };
	}

	const auto& points = drawEntity.GetComponent<ShapeData>().points;

	return util::ComputeBoundingBox(points);
}

void ShapeManager::AddPointToUprightRect(SDL_FPoint p, Shape& rectShape, ShapeData& rectData)
{
	if (rectData.points.empty())
	{
		// first point, just add the point entity and wait for second point to be added
		auto pointStruct = ShapeManager::Point::Create(p, rectData.color);
		assert(pointStruct.IsValid());

		rectData.points.emplace_back(p);
		rectShape.pointEntities.emplace_back(pointStruct);
	}
	else if (rectData.points.size() == 1)
	{
		// second point added, need to fill out all four points
		auto firstPoint = rectData.points[0];
		rectData.points.clear();

		const bool updateFocusPoint = &GetFocusedPoint() == &rectShape.pointEntities[0] &&
									  focusedPoint_.pointIndex != 0;	

		auto firstPointStruct = rectShape.pointEntities[0];
		rectShape.pointEntities.clear();

		auto corners = MakeRectCorners(firstPoint, p);
		for (size_t i = 0; i < corners.size(); ++i)
		{
			auto corner = corners[i];

			Point newPointStruct{};

			if (updateFocusPoint && corner == firstPoint)
			{
				focusedPoint_.pointIndex = i;
				newPointStruct = firstPointStruct;
				newPointStruct.SetPosition(corner); // just in case old pos was slightly different
			}
			else
			{
				newPointStruct = ShapeManager::Point::Create(corner, rectData.color);
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

void ShapeManager::AddPointToPolygon(SDL_FPoint p, Shape& polyShape, ShapeData& polyData)
{
	polyData.points.emplace_back(p);
	polyShape.pointEntities.emplace_back(Point::Create(p, polyData.color));
}


bool ShapeManager::AddPoint(std::string_view name, SDL_FPoint p)
{
	auto* shape = GetShape(name);
	if (!shape)
	{
		return false;
	}

	if (!(shape->drawEntity.IsValid() && shape->drawEntity.HasComponent<ShapeData>()))
	{
		return false;
	}

	auto& data = shape->drawEntity.GetComponent<ShapeData>();
	if (!CanAddMorePointsImpl(data))
	{
		return false;
	}

	switch (data.shapeType)
	{
	case ShapeData::kUprightRect:
		AddPointToUprightRect(p, *shape, data); break;
	case B2Shape::Type::Polygon:
		AddPointToPolygon(p, *shape, data); break;
	default:
		return false;
	}

	return true;
}

//void UpdateUprightRectShape(ShapeManager::Shape& shape, const ShapeData& data,
//							SDL_FPoint newPos)
//{
//	assert(data.shapeType == ShapeData::kUprightRect);
//	assert(data.points.size() == 2);
//	assert(shape.pointEntities.size() == 2);
//	for (size_t i = 0; i < data.points.size(); ++i)
//	{
//		shape.pointEntities[i].SetPosition(data.points[i]);
//	}
//}

void ShapeManager::TransformRectToPolygonShape(Shape& shape, ShapeData& data)
{
	assert(data.shapeType == ShapeData::kUprightRect);
	assert(data.points.size() == 2);
	assert(shape.pointEntities.size() == 2);

	auto corners = MakeRectCorners(data.points[0], data.points[1]);

	std::optional<SDL_FPoint> restorePointPos;
	auto cleanup = [&](size_t idx) {
		auto pos = data.points[idx];
		auto& pointStruct = shape.pointEntities[idx];

		if (&GetFocusedPoint() == &pointStruct)
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
		auto point = Point::Create(data.points[i], data.color);
		assert(point.IsValid());

		shape.pointEntities.emplace_back(point);

		if (restorePointPos.has_value() && 
			EqualsWithTolerance(restorePointPos->x, data.points[i].x) &&
			EqualsWithTolerance(restorePointPos->y, data.points[i].y))
		{
			focusedPoint_.pointIndex = i;
		}
	}

	data.shapeType = B2Shape::Type::Polygon;
}

auto ShapeManager::GetShape(std::string_view name) -> Shape*
{
	if (auto it = shapeIdxByName_.find(name); it != shapeIdxByName_.end())
	{
		assert(it->second < shapes_.size());
		return &shapes_[it->second];
	}
	return nullptr;
}

auto ShapeManager::GetShape(std::string_view name) const -> const Shape*
{
	if (auto it = shapeIdxByName_.find(name); it != shapeIdxByName_.end())
	{
		assert(it->second < shapes_.size());
		return &shapes_[it->second];
	}
	return nullptr;
}

ShapeData* ShapeManager::GetShapeData(std::string_view name) 
{
	if (auto shape = GetShape(name))
	{
		if (shape->drawEntity.IsValid() && shape->drawEntity.HasComponent<ShapeData>())
		{
			return &shape->drawEntity.GetComponent<ShapeData>();
		}
	}
	return nullptr;
}

const ShapeData* ShapeManager::GetShapeData(std::string_view name) const
{
	if (auto shape = GetShape(name))
	{
		if (shape->drawEntity.IsValid() && shape->drawEntity.HasComponent<ShapeData>())
		{
			return &shape->drawEntity.GetComponent<ShapeData>();
		}
	}
	return nullptr;
}

nlohmann::ordered_json 
ShapeManager::SerializeShape(std::string_view name, SDL_FPoint origin) const
{
	nlohmann::ordered_json j;

	if (auto shape = GetShape(name))
	{
		if (shape->drawEntity.IsValid() && shape->drawEntity.HasComponent<ShapeData>())
		{ 
			const auto& data = shape->drawEntity.GetComponent<ShapeData>();
			 
			if (!CanBuildShapeImpl(data))
			{
				LOG_ERROR_FMT("Cannot serialize shape '{}': not enough points to build shape", name);
				return j;
			}

			auto points = data.points;

			if (data.shapeType == B2Shape::Type::Polygon)
			{
				assert(data.tempPoint.has_value());
				points.emplace_back(*data.tempPoint);
			} 

			for (auto& p : points)
			{
				p -= origin;
			}

			j = nlohmann::ordered_json{
				{"name", shape->name},
				{"shapeType", ShapeTypeToString(data.shapeType)},
				{"points", points}
			};
		}
	}
	return j;
}

bool ShapeManager::HasShape(std::string_view name) const
{
	return shapeIdxByName_.contains(name);
}

Entity ShapeManager::MakeTextEntity(SDL_FPoint pos, const GlyphTextWriter& writer)
{
	auto e = ECS::CreateEntity();
	if (!e.IsValid())
	{
		return {};
	}

	e.AddComponent(Transform{ .position = pos });
	e.AddComponent(TextRenderableComponent{
		.writer = writer,
		.formatting = {
			.bounds = { 80, 60 },
			.align = TextAlign::Center,
			.scaleToBounds = true
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

Entity ShapeManager::MakePointCircleIconEntity(SDL_FPoint pos, const Sprite& sprite, SDL_Color clr)
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	e.AddComponent(Transform{ .position = pos });

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

bool ShapeManager::Point::SetPosition(SDL_FPoint newPos)
{
	if (!IsValid())
	{
		return false;
	}

	textEntity.GetComponent<Transform>().position = newPos;
	circleIconEntity.GetComponent<Transform>().position = newPos;

	return true;
}

SDL_FPoint ShapeManager::Point::GetPosition() const
{
	if (!IsValid())
	{
		return { 0.0f, 0.0f };
	}

	return circleIconEntity.GetComponent<Transform>().position;
}


void ShapeManager::Point::SetVisible(bool visible)
{
	textEntity.SetComponentVisibility<TextRenderableComponent>(visible);
	circleIconEntity.SetComponentVisibility<SpriteRenderableComponent>(visible);
}

auto ShapeManager::Point::Create(SDL_FPoint pos, SDL_Color clr) -> Point
{
	return {
		.textEntity = MakeTextEntity(pos, CTX::textWriter),
		.circleIconEntity = MakePointCircleIconEntity(pos, CTX::pointCircleIconSprite, clr)
	};
}

bool ShapeManager::NewShape(std::string_view name, B2Shape::Type type, SDL_Color clr)
{
	if (name.empty() || shapeIdxByName_.contains(name))
	{
		return false;
	}

	const size_t newIdx = shapes_.size();

	auto [it, inserted] = shapeIdxByName_.try_emplace(std::string{name}, newIdx);
	assert(inserted);

	auto& shape = shapes_.emplace_back();
	shape.name = it->first;

	shape.drawEntity = ECS::CreateEntity();
	assert(shape.drawEntity.IsValid());

	shape.drawEntity.AddComponent(ShapeData{
		.shapeType = type,
		.color = clr,
		.ordinal = 0
	});

	return true;
}

bool ShapeManager::CanAddMorePoints(std::string_view name) const
{
	if (auto* data = GetShapeData(name))
	{
		return CanAddMorePointsImpl(*data);
	}
	return false;
}

bool ShapeManager::CanBuildShape(std::string_view name) const
{
	if (auto* data = GetShapeData(name))
	{
		return CanBuildShapeImpl(*data);
	}
	return false;
}

std::vector<std::string_view> ShapeManager::GetShapeNames() const
{
	return std::views::transform(shapes_, [](const Shape& shape) { 
		return shape.name; }) | std::ranges::to<std::vector>();
}

size_t ShapeManager::GetOrdinalIfNewFocus(std::string_view name) const
{
	if (auto it = shapeIdxByName_.find(name); it != shapeIdxByName_.end())
	{
		assert(it->second < shapes_.size());
		auto& ent = shapes_[it->second].drawEntity;

		if (ent.IsValid() && ent.HasComponent<ShapeData>())
		{
			return ent.GetComponent<ShapeData>().ordinal;
		}
	}
	return std::numeric_limits<size_t>::max();
}

bool ShapeManager::HasFocusedPoint() const
{
	if (focusedPoint_.IsValid())
	{
		const auto [shapeIdx, pointIdx] = focusedPoint_;

		if (shapeIdx < shapes_.size())
		{
			const auto& shape = shapes_[shapeIdx];

			if (!shape.IsValid())
			{
				return false;
			}

			const auto& data = shape.drawEntity.GetComponent<ShapeData>();
			if (pointIdx >= data.points.size())
			{
				return false;
			}

			if (pointIdx < shape.pointEntities.size())
			{
				return shape.pointEntities[pointIdx].IsValid();
			}
		}
	}

	return false;
}

auto ShapeManager::GetFocusedPoint() -> Point&
{
	return (HasFocusedPoint())
		? shapes_[focusedPoint_.shapeIndex].pointEntities[focusedPoint_.pointIndex]
		: kInvalidPoint;
}

bool ShapeManager::SetFocusedPointPosition(SDL_FPoint newPos)
{
	if (!HasFocusedPoint())
	{
		return false;
	}

	auto& shape = shapes_[focusedPoint_.shapeIndex];
	auto& data = shape.drawEntity.GetComponent<ShapeData>();

	data.points[focusedPoint_.pointIndex] = newPos;

	auto& pointStruct = shape.pointEntities[focusedPoint_.pointIndex];

	pointStruct.SetPosition(newPos);

	return true;
}

void ShapeManager::UpdateFocusedPoint(const AppState& appState)
{
	//if (!(appState.editMode == EditMode::DraggingPoints ||
	//	  appState.editMode == EditMode::NeutralCommitted))
	//{
	//	focusedPoint_ = {};

	//	return;
	//}

	if (appState.editMode == EditMode::DraggingPoints)
	{		
		assert(HasFocusedPoint());

		float smallestDist = std::numeric_limits<float>::max();
		size_t smallestIdx = 0;

		for (size_t i = 0; i < shapes_[focusedPoint_.shapeIndex].pointEntities.size(); ++i)
		{
			const auto& point = shapes_[focusedPoint_.shapeIndex].pointEntities[i];
			const auto pointPos = point.GetPosition();

			const float dx = appState.mousePos.x - pointPos.x;
			const float dy = appState.mousePos.y - pointPos.y;
			const float dist = dx * dx + dy * dy;

			if (dist < smallestDist)
			{
				smallestDist = dist;
				smallestIdx = i;
			}
		}

		assert(smallestIdx < shapes_[focusedPoint_.shapeIndex].pointEntities.size());

		focusedPoint_.pointIndex = smallestIdx;

		return; 
	}

	for (size_t shapeIdx = 0; shapeIdx < shapes_.size(); ++shapeIdx)
	{
		auto& shape = shapes_[shapeIdx];
		assert(shape.IsValid());

		for (size_t pointIdx = 0; pointIdx < shape.pointEntities.size(); ++pointIdx)
		{
			auto& pointStruct = shape.pointEntities[pointIdx];
			auto& pointCircleIconEnt = pointStruct.circleIconEntity;
			const auto& circleEntTf = pointCircleIconEnt.GetComponent<Transform>();

			if (PointInCircle(appState.mousePos, circleEntTf.position, 
							  kPointCircleColliderRadius / 1.5f))
			{
				focusedPoint_ = {
					.shapeIndex = shapeIdx,
					.pointIndex = pointIdx
				};

				return;
			}
		}
	}

	focusedPoint_ = {};
}

void ShapeManager::ClearFocusedPoint() { focusedPoint_ = {}; }

std::optional<SDL_FPoint> 
ShapeManager::ResolveTempPoint(const ShapeData& data, const AppState& appState,
							   const ShapeState& shapeState) const
{
	using enum AppState::DrawTempPointAs;

	switch (appState.drawTempPointAs)
	{
	case Mouse:
		if (shapeState.isFocus)
		{
			return appState.mousePos;
		}
		break;
	case Connect:
		if (CanCapShapePoints(data))
		{
			assert(!data.points.empty());
			return data.points.front();
		}
		break;
	case None: default:
		break;
	}

	return std::nullopt;
}

bool ShapeManager::ShouldDrawPoint(const Point& pointStruct, const AppState& appState,
								   const ShapeState& shapeState)
{
	if (shapeState.isFocus && appState.committed)
	{
		if (appState.editMode == EditMode::DraggingShape)
		{
			return true;
		} 

		if (&GetFocusedPoint() == &pointStruct)
		{
			return true;
		}

		//if (shapeState.mouseInsideShape)
		//{
		//	if (!HasFocusedPoint())
		//	{
		//		return true;
		//	}
		//}
	}

	return false;
}

void ShapeManager::UpdatePointVisibility(Point& point, const AppState& appState,
									     const ShapeState& shapeState)
{
	auto& pointTextEnt = point.textEntity;
	auto& pointCircleIconEnt = point.circleIconEntity;

	const bool displayPointEx = ShouldDrawPoint(point, appState, shapeState);

	if (displayPointEx)
	{
		const SDL_FPoint drawPos = pointTextEnt.GetComponent<Transform>().position;

		pointTextEnt.GetComponent<TextRenderableComponent>().writer.text =
			std::format("({}, {})", static_cast<int>(drawPos.x), static_cast<int>(drawPos.y));
	}

	point.SetVisible(displayPointEx);
}

void ShapeManager::UpdateShape(Shape& shape, const AppState& appState, const ShapeState& shapeState)
{
	assert(shape.drawEntity.IsValid() && shape.drawEntity.HasComponent<ShapeData>());

	auto& data = shape.drawEntity.GetComponent<ShapeData>();

	data.color.a = shapeState.isFocus ? 255 : 100;

	data.ordinal = shapeState.isFocus
		? 0
		: (shapeState.NewFocusThisFrame() && data.ordinal == 0)
			? shapeState.oldOrdinal  // this shape was previously the active one
			: data.ordinal;

	assert(data.points.size() == shape.pointEntities.size());

	AdjustPoints(shape, data);

	for (size_t i = 0; i < data.points.size(); ++i)
	{
		auto& pointStruct = shape.pointEntities[i];

		assert(pointStruct.IsValid());

		auto& pointTextEnt = pointStruct.textEntity;
		auto& pointCircleIconEnt = pointStruct.circleIconEntity;

		const SDL_FPoint canonicalPointPos = pointCircleIconEnt.GetComponent<Transform>().position;

		data.points[i] = canonicalPointPos;
		pointTextEnt.GetComponent<Transform>().position = canonicalPointPos;

		UpdatePointVisibility(pointStruct, appState, shapeState);
	}

	data.tempPoint = ResolveTempPoint(data, appState, shapeState);
}

void ShapeManager::UpdateShapes(const AppState& appState)
{
	ShapeState shapeState{
		.oldOrdinal = GetOrdinalIfNewFocus(appState.activeShapeName)
	};

	for (auto& shape : shapes_)
	{
		assert(shape.drawEntity.IsValid() && shape.drawEntity.HasComponent<ShapeData>());
		
		shapeState.isFocus = (shape.name == appState.activeShapeName);
		//shapeState.mouseInsideShape = PointInsideRect(shape.GetBoundingBox(), appState.mousePos);

		UpdateShape(shape, appState, shapeState);
	}
}

bool ShapeManager::MoveShape(std::string_view name, SDL_FPoint amount)
{
	if (auto* shape = GetShape(name); shape && shape->IsValid())
	{
		auto& data = shape->drawEntity.GetComponent<ShapeData>();

		assert(data.points.size() == shape->pointEntities.size());

		for (size_t i = 0; i < data.points.size(); ++i)
		{
			data.points[i] += amount;
			shape->pointEntities[i].textEntity.GetComponent<Transform>().position += amount;
			shape->pointEntities[i].circleIconEntity.GetComponent<Transform>().position += amount;
		}

		return true;
	}

	return false;
}

// SELECTED SPRITE
SelectedSprite SelectedSprite::Create()
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	e.AddComponent(Transform{ 
		.position = SDLite::Window().GetLocalCenter<SDL_FPoint>(),
		.scale = kDefaultSpriteScale
	});
	e.AddComponent(SpriteRenderableComponent{});
	e.AddComponent(Tags{});

	return SelectedSprite{ e };
}

void SelectedSprite::Update(Sprite&& sprite, std::string&& spriteName)
{
	assert(sprite.resourceHandle.IsValid());
	assert(!spriteName.empty());

	assert(entity.IsValid());
	assert(entity.HasComponent<SpriteRenderableComponent>());
	assert(entity.HasComponent<Tags>());

	auto& rend = entity.GetComponent<SpriteRenderableComponent>();
	rend.sprite = std::move(sprite);

	auto& tags = entity.GetComponent<Tags>().tags;
	tags.clear();
	tags.insert(std::move(spriteName));
}

bool SelectedSprite::Empty() const
{
	assert(entity.IsValid());
	assert(entity.HasComponent<SpriteRenderableComponent>());

	return !entity.GetComponent<SpriteRenderableComponent>().sprite.resourceHandle.IsValid();
}

void SelectedSprite::Reset()
{
	assert(entity.IsValid());
	assert(entity.HasComponent<SpriteRenderableComponent>());

	entity.GetComponent<SpriteRenderableComponent>().sprite = {};
}

SDL_FPoint SelectedSprite::GetPosition() const
{
	assert(entity.IsValid());
	assert(entity.HasComponent<Transform>());

	return entity.GetComponent<Transform>().position;
}

// SPRITE MANAGER
std::vector<std::string_view> SpriteManager::GetSpriteNames() const
{
	auto scene = CTX::scene.lock();
	if (!scene)
	{
		return {};
	}

	const auto& spriteAtlas = scene->GetTextureRepository().GetSpriteAtlas();

	return sprites_ | std::views::transform([&spriteAtlas](const auto& sp) {
		auto op = spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName>(sp);
		assert(op.has_value());

		return std::string_view{ *op };
	}) | std::ranges::to<std::vector>();
}

Result<Void> ColliderMaker2::Init(SceneFixture::SharedPtr& fixture)
{
	if (!fixture)
	{
		return MAKE_ERROR("Scene fixture was null");
	}
	if (CTX::scene.lock())
	{
		return MAKE_ERROR("Scene was already assigned");
	}

	fixture->GetConfiguration().screenColor = SDLite::kColorWhite;

	if (!ECS::RegisterComponent<ShapeData>())
	{
		return MAKE_ERROR("Failed to register ShapeData component");
	}
	if (!ECS::RegisterComponent<MouseOverPoint>())
	{
		return MAKE_ERROR("Failed to register MouseOverPoint component");
	}

	if (!fixture->IsSystemRegistered<ColliderDrawSystem>())
	{
		fixture->RegisterSystem<ColliderDrawSystem>(Phase::Presentation);
	}

	if (!fixture->IsSystemRegistered<GuiSystem>())
	{
		return MAKE_ERROR("GuiSystem was not registered");
	}

	auto& guiSys = fixture->GetSystem<GuiSystem>();
	guiSys.AddWidget("ColliderMaker", [] {
		ColliderMaker2::Draw();
	});

	// FONT
	auto& fonts = fixture->GetTextureRepository().GetFontAtlas();

	TRY(ResourcePath::Font(kFontFileName), fontPath);

	TRY(fonts.LoadFont(fixture->GetRenderer(), FontDescriptor{
		.fontName = "DefaultFont",
		.filepath = std::move(fontPath),
		.fontSize = kFontSize
	}));

	CTX::textWriter = fonts.GetTextWriter("DefaultFont");
	assert(CTX::textWriter.resourceHandle.IsValid());

	// SHAPE SPRITES
	auto& sprites = fixture->GetTextureRepository().GetSpriteAtlas();

	TRY(ResourcePath::Sprite(ShapeManager::kPointCircleIconSpritePath), pointIconPath);

	TRY(sprites.LoadSprite(fixture->GetRenderer(), {
		.spriteName = "PointCircleIcon",
		.filepath = std::move(pointIconPath)
	}), pointIconSprite);
	assert(pointIconSprite.resourceHandle.IsValid());

	CTX::pointCircleIconSprite = std::move(pointIconSprite);

	CTX::mouseEntity = CTX::MouseEntity::Create(fixture->GetEventBus());
	if (!CTX::mouseEntity.IsValid())
	{
		return MAKE_ERROR("Failed to create mouse entity");
	}

	// SPRITE PICKER
	appState_.activeSprite = SelectedSprite::Create();

	// FINALIZE
	CTX::scene = fixture;

	return kVoid;

}

bool ColliderMaker2::HasActiveShape()
{
	return shapeManager_.HasShape(appState_.activeShapeName);
}

void ColliderMaker2::UpdateAppState()
{
	using Mouse = CTX::Mouse;
	using DrawTempPointAs = AppState::DrawTempPointAs;

	auto* activeShape = shapeManager_.GetShape(appState_.activeShapeName);
	if (!activeShape)
	{
		appState_.committed = false;
		appState_.editMode = EditMode::NoActiveShape;

		return;
	}

	const bool mouseInEditArea = !ImGui::GetIO().WantCaptureMouse;
	if (!mouseInEditArea)
	{
		appState_.drawTempPointAs = DrawTempPointAs::Connect;
		appState_.editMode = (appState_.committed) ? EditMode::NeutralCommitted :
													 appState_.editMode;

		return;
	}

	if (Mouse::IsRightClicked())
	{
		appState_.committed = !appState_.committed;

		appState_.drawTempPointAs = appState_.committed ? DrawTempPointAs::Connect :
														  DrawTempPointAs::Mouse;
	}

	// we know mouse is in edit area

	switch (appState_.editMode)
	{
	case EditMode::NeutralCommitted:

		appState_.drawTempPointAs = DrawTempPointAs::Connect;

		if (Mouse::IsLeftClicked())
		{
			if (shapeManager_.HasFocusedPoint())
			{
				appState_.editMode = EditMode::DraggingPoints;

				return;
			}
			if (PointInsideRect(activeShape->GetBoundingBox(), appState_.mousePos))
			{
				appState_.editMode = EditMode::DraggingShape;

				return;
			}
		}

	case EditMode::SelectingSprite:
		break;

	case EditMode::PlacingPoints:
		if (appState_.committed || !shapeManager_.CanAddMorePoints(appState_.activeShapeName))
		{
			appState_.committed = true;
			appState_.editMode = EditMode::NeutralCommitted;
			appState_.drawTempPointAs = DrawTempPointAs::Connect;

			return;
		}
		if (Mouse::IsLeftClicked())
		{
			shapeManager_.AddPoint(appState_.activeShapeName, appState_.mousePos);

			appState_.committed = appState_.committed ||
				!shapeManager_.CanAddMorePoints(appState_.activeShapeName);
		}

		appState_.drawTempPointAs = appState_.committed ? DrawTempPointAs::Connect :
			DrawTempPointAs::Mouse;

		return;

	case EditMode::DraggingSprite:
		break;

	case EditMode::DraggingPoints: 
		assert(appState_.committed);

		appState_.drawTempPointAs = DrawTempPointAs::Connect;

		if (Mouse::IsLeftReleased())
		{
			appState_.editMode = EditMode::NeutralCommitted;

			return;
		}

		assert(shapeManager_.HasFocusedPoint());
		shapeManager_.SetFocusedPointPosition(appState_.mousePos);

		return;

	case EditMode::DraggingShape:
		assert(appState_.committed);

		appState_.drawTempPointAs = DrawTempPointAs::Connect;

		if (Mouse::IsLeftReleased())
		{
			appState_.editMode = EditMode::NeutralCommitted;
		}
		else
		{
			shapeManager_.MoveShape(appState_.activeShapeName, CTX::Mouse::GetRelPos());
		}

		return;
	}
}

void ColliderMaker2::DrawAvailableShapeButtons()
{
	const auto& shapeNames = shapeManager_.GetShapeNames();
	for (size_t i = 0; i < shapeNames.size(); ++i)
	{
		ImGui::PushID(i);

		const auto& name = shapeNames[i];

		if (ImGui::Button(name.data()))
		{
			appState_.activeShapeName = name;
			appState_.committed = false;
		}

		ImGui::PopID();
	}
}


void ColliderMaker2::DrawAddShapeButtons()
{
	B2Shape::Type newShapeType = B2Shape::Type::Invalid;

	if (ImGui::Button("New Rectangle"))
	{
		newShapeType = ShapeData::kUprightRect;
	}
	if (ImGui::Button("New Polygon"))
	{
		newShapeType = B2Shape::Type::Polygon;
	}

	if (newShapeType != B2Shape::Type::Invalid)
	{
		auto name = std::format("{} {}", ColliderMaker2::kDefaultShapeName,
			shapeManager_.GetShapeNames().size() + 1);

		bool newShapeResult = shapeManager_.NewShape(
			name,
			newShapeType,
			GetRandColor()
		);
		assert(newShapeResult);

		appState_.activeShapeName = name;
		appState_.committed = false;
		appState_.editMode = EditMode::PlacingPoints;
	}
}

void ColliderMaker2::HandleSpriteSelection()
{
	assert(appState_.editMode == EditMode::SelectingSprite);

	auto scene = CTX::scene.lock();
	if (!scene)
	{
		return;
	}

	const bool spriteSelected = spritePicker_.Draw(scene->GetTextureRepository());
	if (!spriteSelected)
	{
		return;
	}

	auto spriteData = spritePicker_.GetSelectedSprite();
	assert(spriteData.has_value());
	assert(spriteData->sprite.resourceHandle.IsValid());

	appState_.activeSprite.Update(std::move(spriteData->sprite), std::move(spriteData->spriteName));

	if (!appState_.activeShapeName.empty())
	{
		if (appState_.committed)
		{
			appState_.editMode = EditMode::NeutralCommitted;
		}
		else
		{
			appState_.editMode = EditMode::PlacingPoints;
		}
	}
	else
	{
		appState_.editMode = EditMode::NoActiveShape;
	}
}

void ColliderMaker2::Draw()
{
	if (!CTX::scene.lock())
	{
		return;
	}

	using enum AppState::DrawTempPointAs;

	const std::string editModeStr = std::format("Edit Mode: {}", 
												EditModeToString(appState_.editMode));
	ImGui::Text(editModeStr.c_str());

	const bool selectingSprite = appState_.editMode == EditMode::SelectingSprite;
	if (!selectingSprite)
	{
		if (ImGui::Button("Set Sprite"))
		{
			appState_.editMode = EditMode::SelectingSprite;
		}
	}
	if (appState_.editMode == EditMode::SelectingSprite)
	{
		HandleSpriteSelection();
	}
	else
	{
		appState_.drawTempPointAs = None;
		appState_.mousePos = CTX::Mouse::GetPos();

		shapeManager_.UpdateFocusedPoint(appState_);

		DrawAvailableShapeButtons();

		DrawAddShapeButtons();

		UpdateAppState();

		shapeManager_.UpdateShapes(appState_);

		if (ImGui::Button("Save"))
		{
			auto saveResult = SaveShapesToFile(kJsonSaveFileName);
			if (!saveResult.Success())
			{
				LOG_ERROR("Failed to save shapes: {}", saveResult.GetError());
			}
		}
	}
}

Result<Void> ColliderMaker2::SaveShapesToFile(std::string_view fileName)
{
	nlohmann::ordered_json j = nlohmann::ordered_json::array();
	for (const auto& name : shapeManager_.GetShapeNames())
	{
		j.push_back(shapeManager_.SerializeShape(name, appState_.activeSprite.GetPosition()));
	}

	auto path = JoinPathsRaw(kJsonDirName, kJsonSaveFileName).string();

	std::ofstream file(path);
	if (!file)
	{
		return MAKE_ERROR_FMT("Failed to open file at path '{}'", path);
	}
	file << j.dump(4);
	return kVoid;
}

} // test

#endif // IMGUI_ENABLED