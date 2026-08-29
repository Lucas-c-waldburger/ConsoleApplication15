//#include "GuiEditComponentBuilders.h"
//
//#if IMGUI_ENABLED
//#include "GuiEditPhysics.h"
//#include "GuiEditPropertyTable.h"
//#include "../InspectorEventPanel.h"
//#include "../gui_edit/GuiEditIncludes.h"
//#include "../../../ecs/Ecs.h"
//#include "../../../ecs/EntityEvents.h"
//#include "../../../ecs/EntityPhysics.h"
//#include "../../../core/Algorithms.h"
//#include "../../../components/RigidBodyComponent.h"
//#include "../../../events/EventBus2.h"
//#include "../../../systems/util/DebugDrawUtils.h"
//#include <filesystem>
//#include "../ColliderEditUtility.h"
//#include "../../demo/collider_maker/ColliderMakerCommon.h"
//
//namespace ui {
//
//namespace {
//
//void DrawHullVector(std::optional<std::vector<SDL_FPoint>>& opVec)
//{
//	PropertyGroup("Hull", [&] {
//		return Property("", opVec, VecArgs{ .minSize = 3 });
//	});
//}
//
//namespace detail {
//
//template <typename> struct event_names_array;
//
//template <template <typename...> class TList, typename...Ts>
//struct event_names_array<TList<Ts...>>
//{
//	static constexpr const char* value[] = {
//		GuiEventName<Ts>::name.data()...
//	};
//};
//
//} // detail
//
//static constexpr auto kEventNames =
//	detail::event_names_array<InspectorEventPanel::GuiEventTypeList>::value;
//
//static const ScriptTableDescriptors kEmptyScriptDataDescriptor{};
//
//int GetCurrentEventNameIndex(std::string_view selectedEvName)
//{
//	if (selectedEvName.empty())
//	{
//		return 0;
//	}
//
//	for (int i = 0; i < InspectorEventPanel::GuiEventTypeList::size; ++i)
//	{
//		if (selectedEvName == std::string_view{ kEventNames[static_cast<size_t>(i)] })
//		{
//			return i;
//		} 
//	}
//
//	return 0;
//}
//
//void DrawEventNames(std::string& selectedEvName)
//{
//	if (selectedEvName.empty())
//	{
//		selectedEvName = kEventNames[0];
//	}
//
//	int cur = GetCurrentEventNameIndex(selectedEvName);
//
//	if (ImGui::Combo("##EvNames", &cur, kEventNames, InspectorEventPanel::GuiEventTypeList::size))
//	{
//		selectedEvName = kEventNames[static_cast<size_t>(cur)];
//	}
//}
//
//size_t GetCurrentRelevantEntityIndex(const std::vector<Entity>& es, Entity_t selected)
//{
//	for (size_t i = 0; i < es.size(); ++i)
//	{
//		if (es[i].GetID() == selected)
//		{
//			return i;
//		}
//	}
//
//	return std::numeric_limits<size_t>::max();
//}
//
//void DrawRelevantEntityList(Entity& e, Entity_t& selectedRelevantEntity)
//{
//	if (selectedRelevantEntity == kInvalidEntity)
//	{
//		selectedRelevantEntity = e.GetID();
//	}
//
//	auto es = ECS::GetAllEntitiesWith<Name, Exclude<InspectorTag>>();
//
//	const size_t curIdx = GetCurrentRelevantEntityIndex(es, selectedRelevantEntity);
//	const auto& curName = (curIdx < es.size()) 
//		? es[curIdx].GetComponent<Name>().value 
//		: Null<std::string>();
//
//	if (ImGui::BeginCombo("##RelEnts", curName.c_str()))
//	{
//		for (size_t i = 0; i < es.size(); ++i)
//		{
//			const auto& name = es[i].GetComponent<Name>().value;
//
//			const bool selected = (curName == name);
//
//			if (ImGui::Selectable(name.c_str(), &selected))
//			{
//				selectedRelevantEntity = es[i].GetID();
//			}
//		}
//
//		ImGui::EndCombo();
//	}
//}
//
//namespace detail {
//
//template <typename> struct connect_script_to_event_dispatch_table;
//
//template <template <typename...> class TList, typename...Ts>
//struct connect_script_to_event_dispatch_table<TList<Ts...>>
//{
//	template <typename EvT>
//	static void call(Entity& e, EventBus& bus, std::string_view selectedScriptFn,
//					 Entity_t selectedRelevantEntity)
//	{
//		auto evs = e.GetEvents(bus);
//
//		evs.OnEventScript<EvT>(selectedScriptFn, 
//			EntityEvents::FilterDef{ .relevantEntity = selectedRelevantEntity});
//	}
//
//	using CallSig = void(*)(Entity&, EventBus&, std::string_view, Entity_t);
//
//	static constexpr CallSig value[] = { &call<Ts>... };
//};
//
//using kConnectScriptToEventDispatchTable = 
//	connect_script_to_event_dispatch_table<InspectorEventPanel::GuiEventTypeList>;
//
//} // detail
//
//void ConnectScriptToEvent(Entity& e, EventBus& bus, std::string_view selectedEvName, 
//						  std::string_view selectedScriptFn, Entity_t selectedRelevantEntity)
//{
//	int evIdx = GetCurrentEventNameIndex(selectedEvName);
//	assert(evIdx >= 0 && evIdx < InspectorEventPanel::GuiEventTypeList::size);
//
//	std::invoke(detail::kConnectScriptToEventDispatchTable::value[static_cast<size_t>(evIdx)],
//		e, bus, selectedScriptFn, selectedRelevantEntity);
//}
//
//} // unnamed
//
//bool GuiEditComponentBuilder<RigidBody>::Draw(Entity& e, B2World& world)
//{
//	assert(e.HasComponent<Transform>());
//
//	isActive_ = true;
//
//	SDL_FPoint pos = bodyParams_.position;
//	if (!manuallySelectingPosition_)
//	{
//		pos = e.GetComponent<Transform>().position;
//		bodyParams_.position = pos;
//	}
//
//	if (!BeginPropertyTable())
//	{
//		return false;
//	}
//
//	PropertyGroup("Body Parameters", [&] {
//
//		Property("Body Type", bodyParams_.bodyType);
//
//		if (Property("Position", pos))
//		{
//			manuallySelectingPosition_ = true;
//			bodyParams_.position = pos;
//		}
//
//		Property("Gravity Scale", bodyParams_.gravityScale, 
//				 DragArgs<float>{ 0.05, 0.0f, 100.0f });
//		Property("Fixed Rotation", bodyParams_.fixedRotation);
//
//		return PropertyEditState::None;
//	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });
//
//	PropertyGroup("Body Limits", [&] {
//		return Property("", bodyLimits_, ImGuiTreeNodeFlags_DefaultOpen);
//	}, { .flags = ImGuiTreeNodeFlags_DefaultOpen });
//	
//	bool built = false;
//
//	ImGui::TableNextRow();
//	ImGui::TableNextColumn();
//
//	if (ImGui::Button("Done"))
//	{		
//		e.RemoveComponent<RigidBody>();
//
//		auto& rb = e.AddComponent(ComponentBuilder<RigidBody>{}
//		.WithBodyParameters(bodyParams_)
//		.WithBodyLimits(bodyLimits_)
//		.Build(world));
//
//		assert(rb.body.GetData().IsValid());
//
//		bodyParams_ = {};
//		bodyLimits_ = {};
//
//		isActive_ = false;	
//		manuallySelectingPosition_ = false;
//		built = true;
//	}
//
//	EndPropertyTable();
//
//	return built;
//}
//
//void GuiEditComponentBuilder<RigidBody>::SetIsActive(bool active)
//{
//	if (!active)
//	{
//		bodyParams_ = {};
//		bodyLimits_ = {};
//	}
//
//	isActive_ = active;
//}
//
//Result<Void> GuiEditComponentBuilder<Collider>::Init(SceneFixture& fixture)
//{
//	return colliderEditUtility_.Init(fixture);
//}
//
//void GuiEditComponentBuilder<Collider>::DrawShapePreview(ReadOnly<B2Body>& roBody, const Camera& cam)
//{
//	auto previewE = ECS::GetEntityByID(shapePreviewEntity_);
//	if (!previewE.IsValid())
//	{
//		previewE = ECS::CreateEntity();
//		previewE.AddComponent<InspectorTag>();
//		previewE.AddComponent<test::ShapeData>();
//		shapePreviewEntity_ = previewE.GetID();
//	}
//
//	assert(previewE.IsValid());
//	assert(previewE.HasComponent<test::ShapeData>());
//
//	auto& shapeData = previewE.GetComponent<test::ShapeData>();
//
//	shapeData.shapeType = shapeParams_.shapeType;
//	shapeData.color = SDLite::kColorOrange;
//
//	switch (shapeParams_.shapeType)
//	{
//	case B2Shape::Type::Polygon:
//	{
//		if (shapeParams_.dimensions.has_value())
//		{
//			auto [w, h] = *shapeParams_.dimensions;
//			shapeData.points = { {0.0f, 0.0f}, {w, 0.0f}, {w, h}, {0.0f, h}, {0.0f, 0.0f} };
//		}
//		else
//		{
//			assert(shapeParams_.hull.has_value());
//			shapeData.points = *shapeParams_.hull;
//			shapeData.points.emplace_back(shapeData.points.front());
//		}
//		break;
//	}
//	case B2Shape::Type::Circle:
//	{
//		assert(shapeParams_.radius.has_value());
//		shapeData.points = util::MakeCirclePerimeterPoints({0.0f, 0.0f}, *shapeParams_.radius);
//		break;
//	}
//	default:
//		break;
//	}
//
//	auto center = roBody.GetData().GetPosition();
//	if (shapeParams_.localPosition.has_value())
//	{
//		center += *shapeParams_.localPosition;
//	}
//	center = cam.WorldToScreen<SDL_FPoint>(center);
//
//	for (auto& p : shapeData.points)
//	{
//		p += center;
//	}
//}
//
//
//bool GuiEditComponentBuilder<Collider>::Draw(Entity& e, ReadOnly<B2Body>& roBody, const Camera& cam)
//{
//	assert(roBody.GetData().IsValid());
//
//	isActive_ = true;
//
//	if (!BeginPropertyTable())
//	{
//		return false;
//	}
//
//	PropertyGroup("Shape Parameters", [&] {
//		Property("Shape Type", shapeParams_.shapeType);
//
//		const bool drawLocalRotation = shapeParams_.shapeType == B2Shape::Type::Polygon;
//		const bool drawRadius = shapeParams_.shapeType == B2Shape::Type::Circle;
//		const bool drawDimensionsAndHull = shapeParams_.shapeType == B2Shape::Type::Polygon;
//		
//		if (drawDimensionsAndHull)
//		{
//			if (!shapeParams_.dimensions.has_value() && !shapeParams_.hull.has_value())
//			{
//				shapeParams_.dimensions.emplace(5.0f, 5.0f);
//			}
//
//			if (shapeParams_.dimensions.has_value() && shapeParams_.hull.has_value())
//			{
//				shapeParams_.dimensions.reset();
//			}
//
//			Property("Dimensions", shapeParams_.dimensions);
//
//			if (shapeParams_.hull.has_value() && shapeParams_.dimensions.has_value())
//			{
//				shapeParams_.hull.reset();
//			}
//			if (shapeParams_.hull.has_value())
//			{
//				while (shapeParams_.hull->size() < 3)
//				{ 
//					shapeParams_.hull->emplace_back(0.0f, 0.0f);
//				}
//			}
//
//			DrawHullVector(shapeParams_.hull);
//		}
//		else
//		{
//			shapeParams_.dimensions.reset();
//			shapeParams_.hull.reset();
//		}
//		
//		if (drawRadius)
//		{
//			if (!shapeParams_.radius.has_value())
//			{
//				shapeParams_.radius = 1.0f;
//			}
//
//			Property("Radius", *shapeParams_.radius);
//		}
//		else
//		{
//			shapeParams_.radius.reset();
//		}
//
//		Property("Local Position", shapeParams_.localPosition);
//
//		if (drawLocalRotation)
//		{
//			Property("Local Rotation", shapeParams_.localRotation);
//		}
//		else
//		{
//			shapeParams_.localRotation.reset();
//		}
//
//		return PropertyEditState::None;
//	});
//
//	PropertyGroup("Collider Settings", [&] {
//		Property("Density", colliderSettings_.density);
//		Property("Friction", colliderSettings_.friction);
//		Property("Restitution", colliderSettings_.restitution);
//
//		PropertyGroup("Enable Events", [&] {
//			Property("Contact", colliderSettings_.enableEvents.contact);
//			Property("Hit", colliderSettings_.enableEvents.hit);
//			Property("Sensor", colliderSettings_.enableEvents.sensor);
//
//			return PropertyEditState::None;
//		});
//
//		Property("Enable Collision", colliderSettings_.enableCollision);
//		Property("Is Sensor", colliderSettings_.isSensor);
//
//		return PropertyEditState::None;
//	});
//
//	bool canBuild = false;
//
//	switch (shapeParams_.shapeType)
//	{
//	case B2Shape::Type::Polygon:
//		canBuild = shapeParams_.hull.has_value() || shapeParams_.dimensions.has_value(); 
//		break;
//	case B2Shape::Type::Circle:
//		canBuild = shapeParams_.radius.has_value();
//	default:
//		break;
//	}
//
//	ImGui::TableNextRow();
//	ImGui::TableNextColumn();
//
//	ImGui::BeginDisabled(!canBuild);
//
//	bool built = false;
//
//	if (ImGui::Button("Done"))
//	{
//		e.RemoveComponent<Collider>();
//
//		auto& col = e.AddComponent(ComponentBuilder<Collider>{}
//		 .WithShapeParameters(shapeParams_)
//		 .WithColliderSettings(colliderSettings_)
//		 .Build(roBody));
//
//		assert(col.shape.GetData().IsValid());
//
//		shapeParams_ = {};
//		colliderSettings_ = {};
//
//		WriteAccessor<B2Body>{}(roBody).SetAwake(true);
//
//		ClearShapePreview();
//		isActive_ = false;
//		built = true;
//	}
//	else if (canBuild)
//	{
//		DrawShapePreview(roBody, cam);
//	}
//
//	ImGui::EndDisabled();
//
//	EndPropertyTable();
//
//	return built;
//}
//
//bool GuiEditComponentBuilder<Collider>::DrawInteractive(Entity& e, ReadOnly<B2Body>& roBody, const Camera& cam)
//{
//	isActive_ = true;
//
//	assert(roBody.GetData().IsValid());
//
//	const auto bodyPos = roBody.GetData().GetPosition();
//	colliderEditUtility_.Draw(shapeParams_, bodyPos, cam);
//
//	if (!BeginPropertyTable())
//	{
//		return false;
//	}
//
//	PropertyGroup("Shape Parameters", [&] {
//		ImGui::BeginDisabled();
//
//		Property("Shape Type", shapeParams_.shapeType);
//		Property("Dimensions", shapeParams_.dimensions);
//		DrawHullVector(shapeParams_.hull);
//		Property("Radius", shapeParams_.radius);
//		Property("Local Position", shapeParams_.localPosition);
//		Property("Local Rotation", shapeParams_.localRotation);
//
//		ImGui::EndDisabled();
//
//		return PropertyEditState::None;
//	});
//
//	PropertyGroup("Collider Settings", [&] {
//		Property("Density", colliderSettings_.density);
//		Property("Friction", colliderSettings_.friction);
//		Property("Restitution", colliderSettings_.restitution);
//
//		PropertyGroup("Enable Events", [&] {
//			Property("Contact", colliderSettings_.enableEvents.contact);
//			Property("Hit", colliderSettings_.enableEvents.hit);
//			Property("Sensor", colliderSettings_.enableEvents.sensor);
//
//			return PropertyEditState::None;
//		});
//
//		Property("Enable Collision", colliderSettings_.enableCollision);
//		Property("Is Sensor", colliderSettings_.isSensor);
//
//		return PropertyEditState::None;
//	});
//
//	ImGui::TableNextRow();
//	ImGui::TableNextColumn();
//
//	const bool canBuild = colliderEditUtility_.CanBuild();
//
//	ImGui::BeginDisabled(!canBuild);
//	
//	bool built = false;
//
//	if (ImGui::Button("Done"))
//	{
//		e.RemoveComponent<Collider>();
//
//		auto& col = e.AddComponent(ComponentBuilder<Collider>{}
//			.WithShapeParameters(shapeParams_)
//			.WithColliderSettings(colliderSettings_)
//			.Build(roBody));
//
//		assert(col.shape.GetData().IsValid());
//
//		shapeParams_ = {};
//		colliderSettings_ = {};
//
//		WriteAccessor<B2Body>{}(roBody).SetAwake(true);
//
//		colliderEditUtility_.Reset();
//		isActive_ = false;
//		built = true;
//	}
//
//	ImGui::EndDisabled();
//
//	EndPropertyTable();
//
//	return built;
//}
//
//void GuiEditComponentBuilder<Collider>::SetIsActive(bool active)
//{
//	if (!active)
//	{
//		shapeParams_ = {};
//		colliderSettings_ = {};
//	}
//
//	isActive_ = active;
//}
//
//bool GuiEditComponentBuilder<CallbackInfo>::Draw(Entity& e, ScriptSystem& scriptSys, EventBus& bus)
//{
//	assert(e.IsValid());
//
//	if (!BeginPropertyTable())
//	{
//		return false;
//	}
//
//	Property("event", [] {
//		DrawEventNames(selectedEventName_);
//		return PropertyEditState::None;
//	});
//
//	if (selectedTableId_ != std::numeric_limits<ScriptTable::TableId>::max() &&
//		!scriptSys.ContainsTable(selectedTableId_))
//	{
//		selectedTableId_ = std::numeric_limits<ScriptTable::TableId>::max();
//		selectedTableFunction_.clear();
//	}
//
//	ScriptFilepathsContext filepathsCtx{ 
//		.scriptTableMap = scriptSys.GetScriptTableMap(),
//		.selectedTableId = selectedTableId_
//	};
//
//	Property("script", [&filepathsCtx] {
//		return GuiEditProperty(filepathsCtx);
//	});
//
//	ScriptTableFunctionNamesContext funcNamesCtx{
//		.scriptTableMap = scriptSys.GetScriptTableMap(),
//		.selectedTableId = selectedTableId_,
//		.selectedTableFunction = selectedTableFunction_
//	};
//
//	Property("function", [&funcNamesCtx] {
//		return GuiEditProperty(funcNamesCtx);
//	});
//
//	Property("relevant entity", [&e]{
//		DrawRelevantEntityList(e, selectedRelevantEntity_);
//		return PropertyEditState::None;
//	});
//
//	ImGui::TableNextRow();
//	ImGui::TableNextColumn();
//
//	bool built = false;
//
//	if (ImGui::Button("Done"))
//	{
//		if (CanAddCallback())
//		{
//			UpdateEntityScriptTable(e, scriptSys);
//
//			ConnectScriptToEvent(e, bus, selectedEventName_, selectedTableFunction_,
//								 selectedRelevantEntity_);
//
//			UpdateEntityCallbackInfo(e, scriptSys);
//
//			ClearSelections();
//
//			built = true;
//		}
//	}
//
//	EndPropertyTable();
//
//	return built;
//}
//
//void GuiEditComponentBuilder<CallbackInfo>::SetIsActive(bool active)
//{
//	if (!active)
//	{
//		//tableNamesToIds_.clear();
//	}
//
//	isActive_ = active;
//}
//
//bool GuiEditComponentBuilder<CallbackInfo>::CanAddCallback()
//{
//	return !selectedEventName_.empty() &&
//		   selectedTableId_ != std::numeric_limits<size_t>::max() &&
//		   !selectedTableFunction_.empty();
//}
//
//void GuiEditComponentBuilder<CallbackInfo>::UpdateEntityScriptTable(Entity& e, 
//																	const ScriptSystem& scriptSys)
//{
//	auto tableView = scriptSys.GetTableView(selectedTableId_);
//	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;
//
//	auto& script = e.AddComponent<Script>();
//	if (script.table.GetTableId() != selectedTableId_)
//	{
//		core::EraseIf(tks, [](const auto& tk) {
//			return tk.type == EntityCallbackToken::Type::Script;
//		});
//
//		auto& callbackInfo = e.AddComponent<CallbackInfo>();
//		callbackInfo.eventNames.clear();
//		callbackInfo.scriptFileNames.clear();
//		callbackInfo.tableFunctionNames.clear();
//	}
//
//	script.table = tableView;
//}
//
//void GuiEditComponentBuilder<CallbackInfo>::UpdateEntityCallbackInfo(Entity& e, const ScriptSystem& scriptSys)
//{
//	auto& callbackInfo = e.GetComponent<CallbackInfo>();
//	callbackInfo.eventNames.emplace_back(selectedEventName_);
//	callbackInfo.tableFunctionNames.emplace_back(selectedTableFunction_);
//
//	const auto& selectedScriptFile = scriptSys.GetTableFilepath(selectedTableId_);
//	assert(!selectedScriptFile.empty());
//
//	auto path = fs::path(selectedScriptFile);
//	assert(fs::exists(path));
//
//	callbackInfo.scriptFileNames.emplace_back(path.filename().string());
//}
//
//void GuiEditComponentBuilder<CallbackInfo>::ClearSelections()
//{
//	selectedEventName_.clear();
//	selectedTableId_ = std::numeric_limits<size_t>::max();
//	selectedTableFunction_.clear();
//	selectedRelevantEntity_ = kInvalidEntity;
//}
//
////void GuiEditComponentBuilder<CallbackInfo>::ReloadTableIdMap(const ScriptSystem& scriptSys)
////{
////	tableFilepathsToIds_.clear();
////
////	for (const auto& [id, tableData] : scriptSys.GetTableDataMap())
////	{
////		tableFilepathsToIds_.try_emplace(tableData.filepath, id);
////	}
////}
//
//} // ui
//
//#endif