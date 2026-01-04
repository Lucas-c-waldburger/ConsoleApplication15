#include "PhysicsEditor.h"

#if IMGUI_ENABLED

#include "../../ecs/Ecs.h"

Result<Void> PhysicsEditor::Init(B2World& world)
{
	using CTX = PhysicsEditorContext;
	if (CTX::world)
	{
		LOG_WARNING("PhysicsEditorContext's world was already assigned");
		return Void{};
	}

	CTX::world = &world;

	return Void{};
}

bool PhysicsEditor::DrawBodyParameters(BodyParameters& params)
{
	bool changed = false;

	int current = static_cast<int>(params.bodyType);

	if (ImGui::Combo("Body Type", &current, kBodyTypeNames,
		IM_ARRAYSIZE(kBodyTypeNames)))
	{
		params.bodyType = static_cast<B2Body::Type>(current);
		changed = true;
	}

	float pos[2] = { params.position.x, params.position.y };
	if (ImGui::DragFloat2("Position", pos, 0.1f))
	{
		params.position.x = pos[0];
		params.position.y = pos[1];
		changed = true;
	}

	if (ImGui::DragFloat("Gravity Scale", &params.gravityScale,
		0.05f, 0.0f, 100.0f))
	{
		changed = true;
	}

	if (ImGui::Checkbox("Fixed Rotation", &params.fixedRotation))
	{
		changed = true;
	}

	return changed;
}

bool PhysicsEditor::DrawBodyLimits(BodyLimits& limits)
{
	bool changed = false;

	ImGui::SeparatorText("Linear Velocity");
	changed |= DrawRange("Linear Velocity", limits.linearVelocity);

	ImGui::SeparatorText("Angular Velocity");
	changed |= DrawRange("Angular Velocity", limits.angularVelocity);

	return changed;
}

bool PhysicsEditor::DrawShapeParameters(B2ShapeParameters& params)
{
	bool changed = false;

	int current = static_cast<int>(params.shapeType);

	if (ImGui::Combo("Shape Type", &current, kShapeTypeNames,
		IM_ARRAYSIZE(kShapeTypeNames)))
	{
		params.shapeType = static_cast<B2Shape::Type>(current);
		// UPDATE WHEN MORE SHAPE TYPES SUPPORTED
		if (params.shapeType != B2Shape::Type::Polygon &&
			params.shapeType != B2Shape::Type::Circle)
		{
			params.shapeType = B2Shape::Type::Polygon;
		}

		changed = true;
	}

	ImGui::BeginDisabled(params.shapeType == B2Shape::Type::Circle);

	ImGui::SeparatorText("Dimensions");
	changed |= DrawOptional("Dimensions", params.dimensions);

	ImGui::SeparatorText("Hull");
	changed |= DrawOptional("Hull", params.hull);

	ImGui::EndDisabled();

	ImGui::SeparatorText("Local Position");
	changed |= DrawOptional("Local Position", params.localPosition);

	ImGui::BeginDisabled(params.shapeType == B2Shape::Type::Circle);
	ImGui::SeparatorText("Local Rotation");
	changed |= DrawOptional("Local Rotation", params.localRotation);

	ImGui::EndDisabled();

	ImGui::SeparatorText("Radius");
	changed |= DrawOptional("Radius", params.radius);

	return changed;
}

bool PhysicsEditor::DrawColliderSettings(ColliderSettings& settings)
{
	bool changed = false;

	changed |= ImGui::DragFloat("density", &settings.density, 0.1f);
	changed |= ImGui::DragFloat("friction", &settings.friction, 0.1f);
	changed |= ImGui::DragFloat("restitution", &settings.restitution, 0.1f);

	changed |= ImGui::Checkbox("enableCollision", &settings.enableCollision);
	changed |= ImGui::Checkbox("isSensor", &settings.isSensor);

	ImGui::SeparatorText("enableEvents");
	ImGui::Indent();

	ImGui::BeginDisabled(settings.isSensor);
	changed |= ImGui::Checkbox("contact", &settings.enableEvents.contact);
	changed |= ImGui::Checkbox("hit", &settings.enableEvents.hit);
	ImGui::EndDisabled();

	ImGui::BeginDisabled(!settings.isSensor);
	changed |= ImGui::Checkbox("sensor", &settings.enableEvents.sensor);
	ImGui::EndDisabled();

	ImGui::Unindent();

	return changed;
}

SimpleGuiTable& PhysicsEditor::GetRigidBodyInfoTable()
{
	static std::optional<SimpleGuiTable> rigidBodyTable;

	if (!rigidBodyTable.has_value())
	{
		rigidBodyTable.emplace("RigidBodyInfo");

		rigidBodyTable->DefineRows("Body Type", "Position", "Mass", "Angle",
			"Gravity Scale", "Linear Velocity", "Angular Velocity", "Handle"
		);
	}

	return *rigidBodyTable;
}

void PhysicsEditor::DrawRigidBodyInfo(const RigidBody& rigidBody)
{
	auto& table = GetRigidBodyInfoTable();

	const auto& body = rigidBody.body.GetData();

	static const char* kBodyTypeNames[] = {
		"Static", "Kinematic", "Dynamic"
	};

	auto pos = body.GetPosition();
	auto linV = body.GetLinearVelocity();
	auto angV = body.GetAngularVelocity();

	table[0].SetValue(kBodyTypeNames[static_cast<size_t>(body.GetBodyType())]);
	table[1].SetValue("[ ", pos.x, ", ", pos.y, " ]");
	table[2].SetValue(body.GetMass());
	table[3].SetValue(body.GetAngle());
	table[4].SetValue(body.GetGravityScale());
	table[5].SetValue("[ ", linV.x, ", ", linV.y, " ]");
	table[6].SetValue(angV);
	table[7].SetValue(body.GetHandle().GetHash());

	table.Draw();
}

SimpleGuiTable& PhysicsEditor::GetColliderInfoTable()
{
	static std::optional<SimpleGuiTable> colliderTable;

	if (!colliderTable.has_value())
	{
		colliderTable.emplace("ColliderInfo");

		colliderTable->DefineRows("Shape Type", "Density", "Friction", "Restitution");
	}

	return *colliderTable;
}

bool PhysicsEditor::ReadyToBuild()
{
	using CTX = PhysicsEditorContext;

	return CTX::bodyParams.has_value() &&
		   CTX::shapeParams.has_value() &&
		   CTX::colliderSettings.has_value();
}

void PhysicsEditor::DrawColliderInfo(const Collider& collider)
{
	auto& table = GetColliderInfoTable();

	const auto& shape = collider.shape.GetData();

	table[0].SetValue(kShapeTypeNames[static_cast<size_t>(shape.GetShapeType())]);
	table[1].SetValue(shape.GetDensity());
	table[2].SetValue(shape.GetFriction());
	table[3].SetValue(shape.GetRestitution());

	table.Draw();
}

bool PhysicsEditor::DrawBuildEditor()
{
	using CTX = PhysicsEditorContext;

	//ImGui::TextUnformatted("RigidBody");
	//ImGui::SameLine();

	if (ImGui::Button("RigidBody Builder"))
	{
		ImGui::OpenPopup("RigidBodyBuilder");
	}
	if (ImGui::BeginPopup("RigidBodyBuilder"))
	{
		if (!CTX::bodyParams.has_value())
		{
			CTX::bodyParams.emplace();
		}

		ImGui::SeparatorText("Body Parameters");
		DrawBodyParameters(*CTX::bodyParams);

		if (ImGui::Button("Done"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::Button("Collider Builder"))
	{
		ImGui::OpenPopup("ColliderBuilder");
	}
	if (ImGui::BeginPopup("ColliderBuilder"))
	{
		if (!CTX::shapeParams.has_value())
		{
			CTX::shapeParams.emplace();
			CTX::shapeParams->shapeType = B2Shape::Type::Polygon;
		}
		if (!CTX::colliderSettings.has_value())
		{
			CTX::colliderSettings.emplace();
		}

		ImGui::SeparatorText("Shape Parameters");
		DrawShapeParameters(*CTX::shapeParams);

		ImGui::SeparatorText("Collider Settings");
		DrawColliderSettings(*CTX::colliderSettings);

		if (ImGui::Button("Done"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	bool built = false;

	ImGui::BeginDisabled(!ReadyToBuild());
	if (ImGui::Button("Build"))
	{
		built = true;
	}
	ImGui::EndDisabled();

	return built;
}

void PhysicsEditor::BuildOnEntity(Entity& e)
{
	using CTX = PhysicsEditorContext;

	assert(CTX::world);
	assert(ReadyToBuild());

	if (e.HasComponent<Collider>())
	{
		auto& shape = WriteAccessor<B2Shape>{}(
			e.GetComponent<Collider>().shape);
		if (shape.IsValid())
		{
			shape.Destroy();
		}
		e.RemoveComponent<Collider>();
	}
	if (e.HasComponent<RigidBody>())
	{
		auto& body = WriteAccessor<B2Body>{}(
			e.GetComponent<RigidBody>().body);
		if (body.IsValid())
		{
			body.Destroy();
		}
		e.RemoveComponent<RigidBody>();
	}

	auto& rigidBody = e.AddComponent(ComponentBuilder<RigidBody>{}
		.WithBodyParameters(*CTX::bodyParams)
		.Build(*CTX::world));

	assert(rigidBody.body.GetData().IsValid());

	auto& collider = e.AddComponent(ComponentBuilder<Collider>{}
		.WithColliderSettings(*CTX::colliderSettings)
		.WithShapeParameters(*CTX::shapeParams)
		.Build(rigidBody.body));

	assert(collider.shape.GetData().IsValid());
}

void PhysicsEditor::DrawWorldEditor()
{
	using CTX = PhysicsEditorContext;

	assert(CTX::world);

	bool changed = false;

	auto gravity = CTX::world->GetGravity();
	float g[2] = { gravity.x, gravity.y };
	if (ImGui::DragFloat2("Gravity", g, 0.1f))
	{
		CTX::world->SetGravity(g[0], g[1]);
		changed = true;
	}
}

#endif