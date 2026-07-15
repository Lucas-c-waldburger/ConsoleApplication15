#include "GuiEditComponentBuilders.h"

#if IMGUI_ENABLED
#include "GuiEditPhysics.h"
#include "GuiEditPropertyTable.h"
#include "../../../ecs/Ecs.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../components/RigidBodyComponent.h"

namespace ui {

namespace {

void DrawHullVector(std::optional<std::vector<SDL_FPoint>>& opVec)
{
	//size_t erasedIdx = std::numeric_limits<size_t>::max();

	//VecArgs args{
	//	.minSize = 3
	//};

	PropertyGroup("Hull", [&] {
		return Property("", opVec, VecArgs{ .minSize = 3 });
	});

	/*if (erasedIdx != std::numeric_limits<size_t>::max())
	{
		if (opVec.has_value())
		{
			auto& vec = *opVec;
			if (vec.size() > 3)
			{
				assert(erasedIdx < vec.size());

				vec.erase(vec.begin() + erasedIdx);
			}
		}
	}
	
	if (ImGui::Button("Add"))
	{
		vec.emplace_back(0.0f, 0.0f);
	}*/

	//bool hasValue = opVec.has_value();
	//size_t erasedIdx = std::numeric_limits<size_t>::max();

	//PropertyGroup("Hull", [&] {
	//	Property("", [&] {
	//		if (ImGui::Checkbox("##Value", &hasValue))
	//		{
	//			if (hasValue && !opVec.has_value())
	//			{
	//				opVec = std::vector<SDL_FPoint>{ {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f} };
	//			}
	//			else if (!hasValue && opVec.has_value())
	//			{
	//				opVec.reset();
	//			}
	//		}
	//		return false;
	//	});
	//	if (hasValue)
	//	{
	//		auto& vec = *opVec;

	//		Property("", [&] {
	//			for (size_t i = 0; i < vec.size(); ++i)
	//			{
	//				auto& p = vec[i];

	//				ImGui::TableNextRow();

	//				ImGui::TableNextColumn();

	//				ImGui::AlignTextToFramePadding();

	//				ImGui::PushID(i);

	//				GuiEditProperties<"X", "Y">(p.x, p.y);

	//				ImGui::TableNextColumn();

	//				ImGui::SetNextItemWidth(-FLT_MIN);

	//				const std::string xBtnLabel = std::format("x##{}", i);

	//				ImGui::BeginDisabled(vec.size() < 3);

	//				if (ImGui::Button(xBtnLabel.c_str()))
	//				{
	//					erasedIdx = i;
	//				}

	//				ImGui::EndDisabled();

	//				ImGui::PopID();
	//			}
	//		});

	//		if (erasedIdx != std::numeric_limits<size_t>::max() && vec.size() > 3)
	//		{
	//			assert(erasedIdx < vec.size());

	//			vec.erase(vec.begin() + erasedIdx);
	//		}

	//		if (ImGui::Button("Add"))
	//		{
	//			vec.emplace_back(0.0f, 0.0f);
	//		}
	//	};

	//	return false;
	//});
}

} // unnamed

bool GuiEditComponentBuilder<RigidBody>::Draw(Entity& e, B2World& world)
{
	assert(e.HasComponent<Transform>());

	isActive_ = true;

	//ImGui::BeginChild("RigidBodyBuilderPanel", ImVec2(0, panelHeight), ImGuiChildFlags_AutoResizeY);

	SDL_FPoint pos = bodyParams_.position;
	if (!manuallySelectingPosition_)
	{
		pos = e.GetComponent<Transform>().position;
		bodyParams_.position = pos;
	}

	if (!BeginPropertyTable())
	{
		//ImGui::EndChild();
		return false;
	}

	PropertyGroup("Body Parameters", [&] {

		Property("Body Type", bodyParams_.bodyType);

		if (Property("Position", pos))
		{
			manuallySelectingPosition_ = true;
			bodyParams_.position = pos;
		}

		Property("Gravity Scale", bodyParams_.gravityScale, 
				 DragArgs<float>{ 0.05, 0.0f, 100.0f });
		Property("Fixed Rotation", bodyParams_.fixedRotation);

		return false;
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });

	PropertyGroup("Body Limits", [&] {
		return Property("", bodyLimits_, ImGuiTreeNodeFlags_DefaultOpen);
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });
	
	bool built = false;

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	if (ImGui::Button("Done"))
	{		
		//if (e.HasComponent<RigidBody>([](const auto& rb) {
		//	return rb.body.GetData().IsValid();
		//}))
		//{
		//	e.RemoveComponent
		//}

		e.RemoveComponent<RigidBody>();

		auto& rb = e.AddComponent(ComponentBuilder<RigidBody>{}
		.WithBodyParameters(bodyParams_)
		.WithBodyLimits(bodyLimits_)
		.Build(world));

		//WriteAccessor<B2Body>{}(rb.body).SetPosition(bodyParams_.position);

		bodyParams_ = {};
		bodyLimits_ = {};

		isActive_ = false;	
		manuallySelectingPosition_ = false;
		built = true;
	}

	EndPropertyTable();

	//ImGui::EndChild();

	return built;
}

void GuiEditComponentBuilder<RigidBody>::SetIsActive(bool active)
{
	if (!active)
	{
		bodyParams_ = {};
		bodyLimits_ = {};
	}

	isActive_ = active;
}

bool GuiEditComponentBuilder<Collider>::Draw(Entity& e, ReadOnly<B2Body>& roBody)
{
	assert(roBody.GetData().IsValid());

	isActive_ = true;

	//ImGui::BeginChild("ColliderBuilderPanel", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

	if (!BeginPropertyTable())
	{
		//ImGui::EndChild();
		return false;
	}

	PropertyGroup("Shape Parameters", [&] {
		Property("Shape Type", shapeParams_.shapeType);

		const bool drawLocalRotation = shapeParams_.shapeType == B2Shape::Type::Polygon;
		const bool drawRadius = shapeParams_.shapeType == B2Shape::Type::Circle;
		const bool drawDimensionsAndHull = shapeParams_.shapeType == B2Shape::Type::Polygon;
		
		if (drawDimensionsAndHull)
		{
			if (!shapeParams_.dimensions.has_value() && !shapeParams_.hull.has_value())
			{
				shapeParams_.dimensions.emplace(5.0f, 5.0f);
			}

			if (shapeParams_.dimensions.has_value() && shapeParams_.hull.has_value())
			{
				shapeParams_.dimensions.reset();
			}

			Property("Dimensions", shapeParams_.dimensions);

			if (shapeParams_.hull.has_value() && shapeParams_.dimensions.has_value())
			{
				shapeParams_.hull.reset();
			}
			if (shapeParams_.hull.has_value())
			{
				while (shapeParams_.hull->size() < 3)
				{
					shapeParams_.hull->emplace_back(0.0f, 0.0f);
				}
			}

			DrawHullVector(shapeParams_.hull);
		}
		else
		{
			shapeParams_.dimensions.reset();
			shapeParams_.hull.reset();
		}
		
		if (drawRadius)
		{
			if (!shapeParams_.radius.has_value())
			{
				shapeParams_.radius = 1.0f;
			}

			Property("Radius", *shapeParams_.radius);
		}
		else
		{
			shapeParams_.radius.reset();
		}

		Property("Local Position", shapeParams_.localPosition);

		if (drawLocalRotation)
		{
			Property("Local Rotation", shapeParams_.localRotation);
		}
		else
		{
			shapeParams_.localRotation.reset();
		}

		return false;
	});

	PropertyGroup("Collider Settings", [&] {
		Property("Density", colliderSettings_.density);
		Property("Friction", colliderSettings_.friction);
		Property("Restitution", colliderSettings_.restitution);

		PropertyGroup("Enable Events", [&] {
			Property("Contact", colliderSettings_.enableEvents.contact);
			Property("Hit", colliderSettings_.enableEvents.hit);
			Property("Sensor", colliderSettings_.enableEvents.sensor);

			return false;
		});

		Property("Enable Collision", colliderSettings_.enableCollision);
		Property("Is Sensor", colliderSettings_.isSensor);

		return false;
	});

	bool canBuild = false;

	switch (shapeParams_.shapeType)
	{
	case B2Shape::Type::Polygon:
		canBuild = shapeParams_.hull.has_value() || shapeParams_.dimensions.has_value(); 
		break;
	case B2Shape::Type::Circle:
		canBuild = shapeParams_.radius.has_value();
	default:
		break;
	}

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGui::BeginDisabled(!canBuild);

	bool built = false;

	if (ImGui::Button("Done"))
	{
		//if (e.HasComponent<Collider>([](const auto& col) {
		//	return col.shape.GetData().IsValid();
		//}))
		//{
		//	WriteAccessor<B2Shape>{}(e.GetComponent<Collider>().shape).Destroy();
		//}

		e.RemoveComponent<Collider>();

		e.AddComponent(ComponentBuilder<Collider>{}
		 .WithShapeParameters(shapeParams_)
		 .WithColliderSettings(colliderSettings_)
		 .Build(roBody));

		shapeParams_ = {};
		colliderSettings_ = {};

		WriteAccessor<B2Body>{}(roBody).SetAwake(true);

		isActive_ = false;
		built = true;
	}

	ImGui::EndDisabled();

	EndPropertyTable();

	//ImGui::EndChild();

	return built;
}

void GuiEditComponentBuilder<Collider>::SetIsActive(bool active)
{
	if (!active)
	{
		shapeParams_ = {};
		colliderSettings_ = {};
	}

	isActive_ = active;
}

} // ui

#endif