#include "ColliderMaker.h"
#include "../../../ecs/Ecs.h"
#include "ColliderDrawSystem.h"

#if IMGUI_ENABLED

namespace test {

using CTX = ColliderMakerContext;
using enum MouseInputSource;

namespace {

static constexpr SDL_Color kEmptyColor = { 0,0,0,0 };

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

template <typename Fn, typename...Args> requires (
	std::invocable<Fn, Args...> && is_result_v<std::invoke_result_t<Fn, Args...>>)
bool TryWithErrorPopup(std::string& errMsgStorage, Fn&& fn, Args&&...args)
{
	if (ImGui::BeginPopupModal("ErrorPopup", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped("%s", errMsgStorage.c_str());
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			errMsgStorage.clear();
		}
		ImGui::EndPopup();
	}

	auto result = std::invoke(fn, std::forward<Args>(args)...);
	if (!result.Success())
	{
		errMsgStorage = result.GetError().GetMessage();
		ImGui::OpenPopup("ErrorPopup");

		return false;
	}

	return true;
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

// SHAPES
ShapeData* CTX::Shapes::GetShapeData(ID id)
{
	if (ids_.contains(id))
	{
		auto ent = ECS::GetEntityByID(id);
		if (ent.IsValid() && ent.HasComponent<ShapeData>())
		{
			return &ent.GetComponent<ShapeData>();
		}
	}
	return nullptr;
}
const ShapeData* CTX::Shapes::GetShapeData(ID id) const
{
	if (ids_.contains(id))
	{
		auto ent = ECS::GetEntityByID(id);
		if (ent.IsValid() && ent.HasComponent<ShapeData>())
		{
			return &ent.GetComponent<ShapeData>();
		}
	}
	return nullptr;
}

auto CTX::Shapes::NewShape(B2Shape::Type type, SDL_Color clr) -> ID
{
	auto e = ECS::CreateEntity();

	e.AddComponent(ShapeData{
		.shapeType = type,
		.color = (clr == kEmptyColor) ? GetRandColor() : clr,
		.ordinal = ids_.size()
	});

	ids_.insert(e.GetID());

	return e.GetID();
}

bool CTX::Shapes::AddPoint(ID id, SDL_FPoint p)
{
	if (auto* data = GetShapeData(id))
	{
		data->points.emplace_back(p);
		return true;
	}
	return false;
}

bool CTX::Shapes::UndoPoint(ID id)
{
	if (auto* data = GetShapeData(id); data->points.size())
	{
		data->points.pop_back();
		return true;
	}
	return false;
}

bool CTX::Shapes::CanAddMorePoints(ID id) const
{
	if (auto* data = GetShapeData(id))
	{
		switch (data->shapeType)
		{
		case ShapeData::kUprightRect:
			return data->points.size() < 2;
		default:
			return true;
		}
	}
	return false;
}

bool CTX::Shapes::CanBuildShape(ID id) const
{
	if (auto* data = GetShapeData(id))
	{
		switch (data->shapeType)
		{
		case ShapeData::kUprightRect:
			return data->points.size() == 2;
		case B2Shape::Type::Polygon:
			return data->points.size() >= 3;
		default:
			return false;
		}
	}
	return false;
}

//void CTX::Shapes::FinalizeShape(ID id) 
//{
//	if (auto* data = GetShapeData(id))
//	{
//		switch (data->shapeType)
//		{
//		case B2Shape::Type::Polygon:
//			return data->points.size() >= 3;
//		default:
//		}
//	}
//}

bool CTX::Shapes::ClearPoints(ID id)
{
	if (auto* data = GetShapeData(id))
	{
		data->points.clear();
		return true;
	}
	return false;
}

std::vector<SDL_FPoint> CTX::Shapes::GetShapePoints(ID id) const
{
	if (auto* data = GetShapeData(id))
	{
		return data->points;
	}
	return {};
}

bool CTX::Shapes::EraseShape(ID id)
{
	if (ids_.contains(id))
	{
		auto e = ECS::GetEntityByID(id);
		e.Destroy();
	}

	return static_cast<bool>(ids_.erase(id));
}

SDL_Color CTX::Shapes::GetShapeColor(ID id) const
{
	if (auto* data = GetShapeData(id)) 
	{
		return data->color;
	}
	return SDLite::kColorWhite;
}

void CTX::Shapes::SetTargetPoint(ID id, std::optional<SDL_FPoint> target)
{
	if (auto* data = GetShapeData(id))
	{
		data->tempPoint = target;
	}
}

std::optional<B2ShapeParameters>
CTX::Shapes::MakeShapeParameters(ID id, SDL_FPoint bodyPos) const
{
	if (auto* data = GetShapeData(id))
	{
		B2ShapeParameters params{
			.shapeType = (data->shapeType == ShapeData::kUprightRect 
				? B2Shape::Type::Polygon
				: data->shapeType)
		};

		switch (data->shapeType)
		{
		case ShapeData::kUprightRect:
		{
			assert(data->points.size() == 2);

			auto rect = ResolvePointsToRect(data->points[0], data->points[1]);

			SDL_FPoint center{
				rect.x + (rect.w / 2.0f),
				rect.y + (rect.h / 2.0f)
			};

			params.dimensions = { rect.w, rect.h };
			params.localPosition = center - bodyPos;

			break;
		}
		case B2Shape::Type::Polygon:
		{
			params.hull = data->points | std::views::transform([bodyPos](const auto& p) {
				return p - bodyPos;
			}) | std::ranges::to<std::vector>();

			break;
		}
		default:
			return std::nullopt;
		}

		return params;
	}

	return std::nullopt;
}

std::optional<SDL_FPoint> CTX::Shapes::GetCenter(ID id) const
{
	if (const auto* data = GetShapeData(id))
	{
		if (data->points.empty())
		{
			return SDL_FPoint{ 0.0f, 0.0f };
		}
		if (data->points.size() == 1)
		{
			return data->points.front();
		}

		switch (data->shapeType)
		{
		case ShapeData::kUprightRect:
		{
			assert(data->points.size() == 2);

			auto rect = ResolvePointsToRect(data->points[0], data->points[1]);

			return SDL_FPoint{
				rect.x + (rect.w / 2.0f),
				rect.y + (rect.h / 2.0f)
			};
		}
		case B2Shape::Type::Polygon:
		{
			return ComputeCentroid(data->points);
		}
		default:
			break;
		}
	}

	return std::nullopt;
}

bool CTX::Shapes::ShapeValid(ID id) const
{
	return ids_.contains(id);
}

B2Shape::Type CTX::Shapes::GetShapeType(ID id) const
{
	if (const auto* data = GetShapeData(id))
	{
		return data->shapeType;
	}
	return B2Shape::Type::Invalid;
}

Result<Void> CTX::AddCollider(Shapes::ID id, Entity& e, 
							  const EntityPhysics::ColliderParams& params)
{
	assert(e.IsValid());

	if (!shapes.ShapeValid(id))
	{
		return MAKE_ERROR("Shape ID invalid");
	}

	auto scene = CTX::scene.lock();
	assert(scene);

	auto phys = e.GetPhysics(scene->GetWorld());

	if (!phys.HasBody())
	{
		auto bodyPos = shapes.GetCenter(id);
		if (!bodyPos.has_value())
		{
			return MAKE_ERROR("Could not find shape center");
		}

		auto body = phys.AddBody(B2Body::Type::Dynamic, *bodyPos);

		assert(body.IsValid());
	}

	auto bodyPos = e.GetComponent<RigidBody>().body.GetData().GetPosition();

	auto sh = shapes.MakeShapeParameters(id, bodyPos);
	if (!sh.has_value())
	{
		return MAKE_ERROR("MakeShapeParameters failed");
	}

	auto colliderEnt = phys.AddCollider(*sh, params);
	assert(colliderEnt.IsValid());
	assert(colliderEnt.GetComponent<Collider>().shape.GetData().IsValid());

	auto& profile = colliderEnt.AddComponent<SpriteRenderableComponent>().profile;
	profile.debugDraw.collider = {
		.on = true,
		.color = shapes.GetShapeColor(id)
	};

	return kVoid;
}

bool ColliderMakerUI::HasActiveShape()
{
	if (activeShape_ != kInvalidEntity)
	{
		assert(CTX::shapes.GetShapeIDs().contains(activeShape_));
		return true;
	}
	return false;
}

Result<Void> ColliderMakerUI::Init(SceneFixture::SharedPtr& fixture, Entity& e)
{
	if (!fixture)
	{
		return MAKE_ERROR("Scene fixture was null");
	}
	if (CTX::scene.lock())
	{
		return MAKE_ERROR("Scene was already assigned");
	}

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
	guiSys.AddWidget("ColliderMaker", [e] mutable {
		if (e.IsValid())
		{
			ColliderMakerUI::Draw(e);
		}
	});


	CTX::scene = fixture;

	return kVoid;
}

void ColliderMakerUI::Draw(Entity& e)
{
	const auto& shapeIds = CTX::shapes.GetShapeIDs();
	int count = 0;
	for (const auto& shapeId : shapeIds)
	{
		ImGui::PushID(count);

		std::string id = std::format("Shape {}", count);

		if (ImGui::Button(id.c_str()))
		{
			activeShape_ = shapeId;
		}

		ImGui::PopID();
		++count;
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
		activeShape_ = CTX::shapes.NewShape(newShapeType);	
		committed_ = false;
	}

	if (!ImGui::GetIO().WantCaptureMouse && HasActiveShape())
	{		
		if (!committed_)
		{
			auto mousePos = CTX::Mouse::GetPos();

			CTX::shapes.SetTargetPoint(activeShape_, mousePos);

			if (CTX::Mouse::IsLeftClicked())
			{
				if (CTX::shapes.CanAddMorePoints(activeShape_))
				{
					CTX::shapes.AddPoint(activeShape_, mousePos);
				}
			}

			if (CTX::Mouse::IsRightClicked() || !CTX::shapes.CanAddMorePoints(activeShape_))
			{
				committed_ = true;

				if (CTX::shapes.CanBuildShape(activeShape_))
				{
					if (CTX::shapes.GetShapeType(activeShape_) == B2Shape::Type::Polygon)
					{
						CTX::shapes.SetTargetPoint(
							activeShape_,
							CTX::shapes.GetShapePoints(activeShape_).front()
						);
					}
					else
					{
						CTX::shapes.SetTargetPoint(activeShape_, {});
					}
				}
			}
		}
	}
	else
	{
		if (CTX::shapes.CanBuildShape(activeShape_))
		{
			if (CTX::shapes.GetShapeType(activeShape_) == B2Shape::Type::Polygon)
			{
				CTX::shapes.SetTargetPoint(
					activeShape_,
					CTX::shapes.GetShapePoints(activeShape_).front()
				);
			}
			else
			{
				CTX::shapes.SetTargetPoint(activeShape_, {});
			}
		}
	}

	if (ImGui::Button("Commit"))
	{
		if (committed_ && CTX::shapes.CanBuildShape(activeShape_))
		{
			TryWithErrorPopup(CTX::currentErrorMsg, [&e] {
				auto result = CTX::AddCollider(activeShape_, e, {});

				CTX::shapes.EraseShape(activeShape_);
				activeShape_ = kInvalidEntity;
				committed_ = false;

				return result;
			});
		}
	}
}

void ColliderMakerUI::Cleanup()
{
	for (const auto id : CTX::shapes.GetShapeIDs())
	{
		CTX::shapes.EraseShape(id);
	}
}

} // test

#endif