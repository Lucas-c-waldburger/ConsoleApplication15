#include "ColliderMaker2.h"
#include "ColliderDrawSystem.h"
#include <format>
#include "../../../serial/user_types/SDLJsonUserTypes.h"

#if IMGUI_ENABLED

namespace test {

using CTX = ColliderMaker2Context;

using enum MouseInputSource;

namespace {

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
		return data.points.size() == 2;
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
		return true;
	}
}

} // unnamed

// MOUSE
SDL_FPoint CTX::Mouse::GetPos()
{
	return GetMouse().values.cursor.absolutePos;
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

bool ShapeManager::AddPoint(std::string_view name, SDL_FPoint p, const GlyphTextWriter& writer)
{
	if (auto* shape = GetShape(name))
	{
		if (shape->drawEntity.IsValid() && shape->drawEntity.HasComponent<ShapeData>())
		{
			auto& data = shape->drawEntity.GetComponent<ShapeData>();
			if (!CanAddMorePointsImpl(data))
			{
				return false;
			}

			data.points.emplace_back(p);

			auto pointTextEnt = MakePointEntity(p, writer);
			if (!pointTextEnt.IsValid())
			{
				LOG_ERROR_FMT("Failed to create point text entity for new point in shape '{}'", name);
				return false;
			}

			shape->pointTextEntities.emplace_back(pointTextEnt);

			return true;
		}
	}
	return false;
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

nlohmann::ordered_json ShapeManager::SerializeShape(std::string_view name) const
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

			j = nlohmann::ordered_json{
				{"name", shape->name},
				{"shapeType", data.shapeType},
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

Entity ShapeManager::MakePointEntity(SDL_FPoint pos, const GlyphTextWriter& writer)
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
			.bounds = { 40, 40 },
			.align = TextAlign::Center,
			.scaleToBounds = true
		},
		.profile = {
			.drawOrder = 1000,
			.mods = {
				.color = RGB::FromSDLColor(kTextColor)
			},
			.offset = { 5.0f, 5.0f }
		}
	});

	return e;
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

void ShapeManager::UpdateShapeData(Shape& shape, const FocusState& state,
								   const std::optional<SDL_FPoint>& mousePos)
{
	assert(shape.drawEntity.IsValid() && shape.drawEntity.HasComponent<ShapeData>());

	auto& data = shape.drawEntity.GetComponent<ShapeData>();

	data.color.a = state.isFocus ? 255 : 100;

	data.ordinal = state.isFocus
		? 0
		: (state.NewFocusThisFrame() && data.ordinal == 0)
			? state.oldOrdinal  // was previously the current shape
			: data.ordinal;

	assert(data.points.size() == shape.pointTextEntities.size());

	for (size_t i = 0; i < data.points.size(); ++i)
	{
		auto& pointTextEnt = shape.pointTextEntities[i];
		auto& drawPoint = data.points[i];

		assert(pointTextEnt.IsValid() && pointTextEnt.HasComponent<Transform>());
		
		drawPoint = pointTextEnt.GetComponent<Transform>().position;

		pointTextEnt.SetComponentVisibility<TextRenderableComponent>(state.isFocus);
	}

	data.tempPoint.reset();

	if (CanAddMorePointsImpl(data))
	{
		if (state.isFocus)
		{
			data.tempPoint = mousePos;
		}
	}
	else 
	{
		data.tempPoint = data.points.front();
	}
}

void ShapeManager::FrameUpdate(std::string_view currentShapeName, 
							   const std::optional<SDL_FPoint>& mousePos)
{
	FocusState focusState{
		.oldOrdinal = GetOrdinalIfNewFocus(currentShapeName)
	};

	for (auto& shape : shapes_)
	{
		assert(shape.drawEntity.IsValid() && shape.drawEntity.HasComponent<ShapeData>());
		
		focusState.isFocus = (shape.name == currentShapeName);

		UpdateShapeData(shape, focusState, mousePos);		
	}
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

	ECS::RegisterComponent<ShapeData>();

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

	auto& fonts = fixture->GetTextureRepository().GetFontAtlas();

	TRY(ResourcePath::Font(kFontFileName), fontPath);

	TRY(fonts.LoadFont(fixture->GetRenderer(), FontDescriptor{
		.fontName = "DefaultFont",
		.filepath = std::move(fontPath),
		.fontSize = kFontSize
	}));

	CTX::textWriter = fonts.GetTextWriter("DefaultFont");
	assert(CTX::textWriter.resourceHandle.IsValid());

	CTX::scene = fixture;

	return kVoid;

}

bool ColliderMaker2::HasActiveShape()
{
	return shapeManager_.HasShape(currentShapeName_);
}

void ColliderMaker2::Draw()
{
	if (!CTX::scene.lock())
	{
		return;
	}

	const auto& shapeNames = shapeManager_.GetShapeNames();
	for (size_t i = 0; i < shapeNames.size(); ++i)
	{
		ImGui::PushID(i);

		const auto& name = shapeNames[i];

		if (ImGui::Button(name.data()))
		{
			currentShapeName_ = name;
			committed_ = false;
		}

		ImGui::PopID();
	}

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
		auto name = std::format("{} {}", ColliderMaker2::kDefaultShapeName, shapeNames.size() + 1);

		bool newShapeResult = shapeManager_.NewShape(
			name,
			newShapeType,
			GetRandColor()
		);
		assert(newShapeResult);

		if (currentShapeName_.empty())
		{
			currentShapeName_ = name;
		}
	}

	std::optional<SDL_FPoint> mousePos;

	const bool mouseInEditArea = !ImGui::GetIO().WantCaptureMouse;

	bool retrieveMouse = false;
	bool addPoint = false;

	if (!committed_ && mouseInEditArea && HasActiveShape())
	{
		retrieveMouse = true;

		const bool canAddPoints = shapeManager_.CanAddMorePoints(currentShapeName_);

		if (CTX::Mouse::IsLeftClicked() && canAddPoints)
		{
			addPoint = true;
		}

		else if (CTX::Mouse::IsRightClicked() || !canAddPoints)
		{
			retrieveMouse = false;
			committed_ = true;
		}
	}

	if (retrieveMouse)
	{
		mousePos = CTX::Mouse::GetPos();
	}

	if (addPoint)
	{
		assert(mousePos.has_value());

		shapeManager_.AddPoint(currentShapeName_, *mousePos,
			ColliderMaker2Context::textWriter);
	}

	shapeManager_.FrameUpdate(currentShapeName_, mousePos);

	if (ImGui::Button("Save"))
	{
		auto saveResult = SaveShapesToFile(kJsonSaveFileName);
		if (!saveResult.Success())
		{
			LOG_ERROR("Failed to save shapes: {}", saveResult.GetError());
		}
	}
}

Result<Void> ColliderMaker2::SaveShapesToFile(std::string_view fileName)
{
	nlohmann::ordered_json j = nlohmann::ordered_json::array();
	for (const auto& name : shapeManager_.GetShapeNames())
	{
		j.push_back(shapeManager_.SerializeShape(name));
	}

	TRY(ResourcePath::Json(fileName), path);

	std::ofstream file(path.data());
	if (!file)
	{
		return MAKE_ERROR(std::format("Failed to open file at path '{}'", path));
	}
	file << j.dump(4);
	return kVoid;
}

} // test

#endif // IMGUI_ENABLED