#include "GuiEditComponentBuilders.h"

#if IMGUI_ENABLED
#include "GuiEditPhysics.h"
#include "GuiEditPropertyTable.h"
#include "../InspectorEventPanel.h"
#include "../gui_edit/GuiEditIncludes.h"
#include "../../../ecs/Ecs.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../core/Algorithms.h"
#include "../../../components/RigidBodyComponent.h"
#include "../../../events/EventBus2.h"
#include <filesystem>

namespace ui {

namespace {

void DrawHullVector(std::optional<std::vector<SDL_FPoint>>& opVec)
{
	PropertyGroup("Hull", [&] {
		return Property("", opVec, VecArgs{ .minSize = 3 });
	});
}

namespace detail {

template <typename> struct event_names_array;

template <template <typename...> class TList, typename...Ts>
struct event_names_array<TList<Ts...>>
{
	static constexpr const char* value[] = {
		GuiEventName<Ts>::name.data()...
	};
};

} // detail

static constexpr auto kEventNames =
	detail::event_names_array<InspectorEventPanel::GuiEventTypeList>::value;

static const ScriptDataDescriptor kEmptyScriptDataDescriptor{};

int GetCurrentEventNameIndex(std::string_view selectedEvName)
{
	if (selectedEvName.empty())
	{
		return 0;
	}

	for (int i = 0; i < InspectorEventPanel::GuiEventTypeList::size; ++i)
	{
		if (selectedEvName == std::string_view{ kEventNames[static_cast<size_t>(i)] })
		{
			return i;
		} 
	}

	return 0;
}

void DrawEventNames(std::string_view& selectedEvName)
{
	int cur = GetCurrentEventNameIndex(selectedEvName);

	if (ImGui::Combo("##EvNames", &cur, kEventNames, InspectorEventPanel::GuiEventTypeList::size))
	{
		selectedEvName = kEventNames[static_cast<size_t>(cur)];
	}
}

namespace detail {

template <typename> struct connect_script_to_event_dispatch_table;

template <template <typename...> class TList, typename...Ts>
struct connect_script_to_event_dispatch_table<TList<Ts...>>
{
	template <typename EvT>
	static void call(Entity& e, EventBus& bus, std::string_view selectedScriptFn)
	{
		auto evs = e.GetEvents(bus);

		evs.OnEventScript<EvT>(selectedScriptFn);
	}

	using CallSig = void(*)(Entity&, EventBus&, std::string_view);

	static constexpr CallSig value[] = { &call<Ts>... };
};

using kConnectScriptToEventDispatchTable = 
	connect_script_to_event_dispatch_table<InspectorEventPanel::GuiEventTypeList>;

} // detail

void ConnectScriptToEvent(Entity& e, EventBus& bus, std::string_view selectedEvName, 
						  std::string_view selectedScriptFn)
{
	int evIdx = GetCurrentEventNameIndex(selectedEvName);
	assert(evIdx >= 0 && evIdx < InspectorEventPanel::GuiEventTypeList::size);

	std::invoke(detail::kConnectScriptToEventDispatchTable::value[static_cast<size_t>(evIdx)],
		e, bus, selectedScriptFn);
}

} // unnamed

bool GuiEditComponentBuilder<RigidBody>::Draw(Entity& e, B2World& world)
{
	assert(e.HasComponent<Transform>());

	isActive_ = true;

	SDL_FPoint pos = bodyParams_.position;
	if (!manuallySelectingPosition_)
	{
		pos = e.GetComponent<Transform>().position;
		bodyParams_.position = pos;
	}

	if (!BeginPropertyTable())
	{
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

		return PropertyEditState::None;
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });

	PropertyGroup("Body Limits", [&] {
		return Property("", bodyLimits_, ImGuiTreeNodeFlags_DefaultOpen);
	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });
	
	bool built = false;

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	if (ImGui::Button("Done"))
	{		
		e.RemoveComponent<RigidBody>();

		auto& rb = e.AddComponent(ComponentBuilder<RigidBody>{}
		.WithBodyParameters(bodyParams_)
		.WithBodyLimits(bodyLimits_)
		.Build(world));

		bodyParams_ = {};
		bodyLimits_ = {};

		isActive_ = false;	
		manuallySelectingPosition_ = false;
		built = true;
	}

	EndPropertyTable();

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

	if (!BeginPropertyTable())
	{
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

		return PropertyEditState::None;
	});

	PropertyGroup("Collider Settings", [&] {
		Property("Density", colliderSettings_.density);
		Property("Friction", colliderSettings_.friction);
		Property("Restitution", colliderSettings_.restitution);

		PropertyGroup("Enable Events", [&] {
			Property("Contact", colliderSettings_.enableEvents.contact);
			Property("Hit", colliderSettings_.enableEvents.hit);
			Property("Sensor", colliderSettings_.enableEvents.sensor);

			return PropertyEditState::None;
		});

		Property("Enable Collision", colliderSettings_.enableCollision);
		Property("Is Sensor", colliderSettings_.isSensor);

		return PropertyEditState::None;
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

bool GuiEditComponentBuilder<CallbackInfo>::Draw(Entity& e, ScriptSystem& scriptSys, EventBus& bus)
{
	assert(e.IsValid());

	if (!BeginPropertyTable())
	{
		return false;
	}

	Property("event", [] {
		DrawEventNames(selectedEventName_);
		return PropertyEditState::None;
	});

	auto package = scriptSys.ExportScriptDataPackage();

	ScriptFilepathsContext filepathsCtx{
		.package = package,
		.selectedFilepath = selectedScriptFile_
	};

	Property("script", [&filepathsCtx] {
		return GuiEditProperty(filepathsCtx);
	});

	ScriptTableFunctionNamesContext funcNamesCtx{
		.tableFunctionNames = (filepathsCtx.packageIndex < package.size()
			? package[filepathsCtx.packageIndex].tableDescriptor.functionNames
			: Null<std::vector<std::string>>()),
		.selectedTableFunction = selectedTableFunction_
	};

	Property("function", [&funcNamesCtx] {
		return GuiEditProperty(funcNamesCtx);
	});

	if (ImGui::Button("Done") && CanAddCallback())
	{
		UpdateEntityScriptTable(e, scriptSys);

		ConnectScriptToEvent(e, bus, selectedEventName_, selectedTableFunction_);

		UpdateEntityCallbackInfo(e);

		ClearSelections();

		return true;
	}

	EndPropertyTable();

	return false;
}

void GuiEditComponentBuilder<CallbackInfo>::SetIsActive(bool active)
{
	if (!active)
	{
		//tableNamesToIds_.clear();
	}

	isActive_ = active;
}

bool GuiEditComponentBuilder<CallbackInfo>::CanAddCallback()
{
	return !selectedEventName_.empty() &&
		   !selectedScriptFile_.empty() &&
		   !selectedTableFunction_.empty();
}

void GuiEditComponentBuilder<CallbackInfo>::UpdateEntityScriptTable(Entity& e, 
																	const ScriptSystem& scriptSys)
{
	const auto& tableData = scriptSys.GetTableDataMap();
	auto it = core::FindIf(tableData, [](const auto& pair) {
		return pair.second.filepath == selectedScriptFile_;
	});

	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;

	if (it != tableData.end())
	{
		auto& script = e.AddComponent<Script>();
		if (script.table.GetTableId() != it->first)
		{
			core::EraseIf(tks, [](const auto& tk) {
				return tk.type == EntityCallbackToken::Type::Script;
			});

			auto& callbackInfo = e.AddComponent<CallbackInfo>();
			callbackInfo.eventNames.clear();
			callbackInfo.scriptFileNames.clear();
			callbackInfo.tableFunctionNames.clear();
		}

		script.table = it->second.table.GetView();
	}
}

void GuiEditComponentBuilder<CallbackInfo>::UpdateEntityCallbackInfo(Entity& e)
{
	auto& callbackInfo = e.GetComponent<CallbackInfo>();
	callbackInfo.eventNames.emplace_back(selectedEventName_);
	callbackInfo.tableFunctionNames.emplace_back(selectedTableFunction_);

	auto path = fs::path(selectedScriptFile_);
	assert(fs::exists(path));

	callbackInfo.scriptFileNames.emplace_back(path.filename().string());
}

void GuiEditComponentBuilder<CallbackInfo>::ClearSelections()
{
	selectedEventName_ = {};
	selectedScriptFile_.clear();
	selectedTableFunction_.clear();
}

//void GuiEditComponentBuilder<CallbackInfo>::ReloadTableIdMap(const ScriptSystem& scriptSys)
//{
//	tableFilepathsToIds_.clear();
//
//	for (const auto& [id, tableData] : scriptSys.GetTableDataMap())
//	{
//		tableFilepathsToIds_.try_emplace(tableData.filepath, id);
//	}
//}

} // ui

#endif