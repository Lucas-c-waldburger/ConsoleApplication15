#include "ControllerMappingEditor.h"
#include "../../ecs/Ecs.h"
#include "../../events/EventBus2.h"
#include "../callbacks/GameControllerCallbacks.h"

Result<Void> ControllerMappingEditor::Init(const GameControllerEventHandler& handler)
{
	using CTX = ControllerMappingEditorContext;
	if (CTX::controllerEventHandler)
	{
		LOG_WARNING("ControllerMappingEditorContext's controller event handler "
			"was already assigned");
		return Void{};
	}

	CTX::controllerEventHandler = &handler;

	return Void{};
}

bool ControllerMappingEditor::IsInitialized()
{ 
	return ControllerMappingEditorContext::controllerEventHandler != nullptr; 
}

bool ControllerMappingEditor::DrawBodyLimits(BodyLimits& limits)
{
	bool changed = false;

	ImGui::SeparatorText("Linear Velocity");
	changed |= DrawRange("Linear Velocity", limits.linearVelocity);

	ImGui::SeparatorText("Angular Velocity");
	changed |= DrawRange("Angular Velocity", limits.angularVelocity);

	float imp[2] = { limits.maxImpulse.x, limits.maxImpulse.y };
	if (ImGui::DragFloat2("Max Impulse", imp))
	{
		limits.maxImpulse.x = imp[0];
		limits.maxImpulse.y = imp[1];
		changed = true;
	}

	return changed;
}

bool ControllerMappingEditor::ConnectEntityToFirstController(Entity& e)
{
	using CTX = ControllerMappingEditorContext;
	assert(CTX::controllerEventHandler);

	if (IsEntityConnectedToController(e))
	{
		return false;
	}

	auto& controllerState = e.AddComponent<GameControllerState>();

	// try to connect to existing active controller
	const auto& activeControllers = CTX::controllerEventHandler->GetActiveControllers();
	if (!activeControllers.empty())
	{
		for (const auto& [_, pair] : activeControllers)
		{
			if (pair.first.IsConnected())
			{
				controllerState.joystickID = pair.first.GetJoystickID();
				return true;
			}
		}
	}

	return false;
}

void ControllerMappingEditor::DrawControllerMappingEditor(Entity& e, EventBus2& bus)
{
	using CTX = ControllerMappingEditorContext;
	assert(CTX::controllerEventHandler);

	if (!IsEntityConnectedToController(e))
	{
		if (!ConnectEntityToFirstController(e))
		{
			ImGui::Text("Waiting for controller connection...");
			return;
		}
	}

	assert(e.HasComponent<RigidBody>());
	auto& rigid = e.GetComponent<RigidBody>();

	auto& controllerState = e.GetComponent<GameControllerState>();

	ImGui::LabelText("Controller Joystick ID", "%i", controllerState.joystickID);

	if (!CTX::axisImpulseSignalToken.IsConnected())
	{
		if (ImGui::Button("Add Axis Movement"))
		{
			CTX::axisImpulseSignalToken = bus.ConnectToInput(
				GameControllerInputSource::LeftStickAxis,
				test::ApplyAxisInputToForce(e)
			);

			assert(CTX::axisImpulseSignalToken.IsConnected());
		}
	}

	if (CTX::axisImpulseSignalToken.IsConnected())
	{
		ImGui::SeparatorText("Movement Body Limits");
		DrawBodyLimits(rigid.limits);
	}
}

void ControllerMappingEditor::DrawControllerState(Entity& e)
{
	//using CTX = ControllerMappingEditorContext;

	//if (!IsEntityConnectedToController(e))
	//{
	//	return;
	//}

	//SDL_JoystickID joystickId = e.GetComponent<GameControllerState>().joystickID;
	//
	//const auto& controllerState = 
	//	CTX::controllerEventHandler->GetControllerState(joystickId);

	//for (size_t i = 0; i < IM_ARRAYSIZE(kControllerInputSourceNames); i++)
	//{
	//	ImGui::LabelText(kControllerInputSourceNames[i]);
	//}
}

bool ControllerMappingEditor::IsEntityConnectedToController(const Entity& e)
{
	using CTX = ControllerMappingEditorContext;
	assert(CTX::controllerEventHandler);

	if (!e.HasComponent<GameControllerState>())
	{
		return false;
	}

	const auto& controllerState = e.GetComponent<GameControllerState>();
	if (controllerState.joystickID == -1)
	{
		return false;
	}

	const auto& activeControllers = CTX::controllerEventHandler->GetActiveControllers();

	auto it = activeControllers.find(controllerState.joystickID);

	return it != activeControllers.end() && it->second.first.IsConnected();
}
