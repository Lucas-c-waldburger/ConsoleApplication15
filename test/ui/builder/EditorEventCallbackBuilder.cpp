#include "EditorEventCallbackBuilder.h"

#if IMGUI_ENABLED
#include <filesystem>
#include "../InspectorEventPanel.h"
#include "../gui_edit/GuiEditEvents.h"
#include "../gui_edit/GuiEventNames.h"
#include "../gui_edit/GuiEditPropertyTable.h"
#include "../../../ecs/EntityEvents.h"

namespace ui {

namespace fs = std::filesystem;

namespace {

/** @defgroup Event Names @{ */
namespace detail {

template <typename> struct event_names_array;

template <template <typename...> class TList, typename...Ts>
struct event_names_array<TList<Ts...>>
{
	static constexpr const char* value[] = { GuiEventName<Ts>::name.data()... };
};

} // detail

static constexpr auto kEventNames =
	detail::event_names_array<InspectorEventPanel::GuiEventTypeList>::value;

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

void DrawEventNames(std::string& selectedEvName)
{
	if (selectedEvName.empty())
	{
		selectedEvName = kEventNames[0];
	}

	int cur = GetCurrentEventNameIndex(selectedEvName);

	if (ImGui::Combo("##EvNames", &cur, kEventNames, InspectorEventPanel::GuiEventTypeList::size))
	{
		selectedEvName = kEventNames[static_cast<size_t>(cur)];
	}
}

size_t GetCurrentRelevantEntityIndex(const std::vector<Entity>& es, Entity_t selected)
{
	for (size_t i = 0; i < es.size(); ++i)
	{
		if (es[i].GetID() == selected)
		{
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

void DrawRelevantEntityList(Entity& e, Entity_t& selectedRelevantEntity)
{
	if (selectedRelevantEntity == kInvalidEntity)
	{
		selectedRelevantEntity = e.GetID();
	}

	auto es = ECS::GetAllEntitiesWith<Name, Exclude<InspectorTag>>();

	const size_t curIdx = GetCurrentRelevantEntityIndex(es, selectedRelevantEntity);
	const auto& curName = (curIdx < es.size())
		? es[curIdx].GetComponent<Name>().value
		: Null<std::string>();

	if (ImGui::BeginCombo("##RelEnts", curName.c_str()))
	{
		for (size_t i = 0; i < es.size(); ++i)
		{
			const auto& name = es[i].GetComponent<Name>().value;

			const bool selected = (curName == name);

			if (ImGui::Selectable(name.c_str(), &selected))
			{
				selectedRelevantEntity = es[i].GetID();
			}
		}

		ImGui::EndCombo();
	}
}

/** @} */

/** @defgroup Connect script dispatch @{ */

namespace detail {

template <typename> struct connect_script_to_event_dispatch_table;

template <template <typename...> class TList, typename...Ts>
struct connect_script_to_event_dispatch_table<TList<Ts...>>
{
	template <typename EvT>
	static void call(Entity& e, EventBus& bus, std::string_view selectedScriptFn,
					 Entity_t selectedRelevantEntity)
	{
		auto evs = e.GetEvents(bus);

		evs.OnEventScript<EvT>(selectedScriptFn,
			EntityEvents::FilterDef{ .relevantEntity = selectedRelevantEntity });
	}

	using CallSig = void(*)(Entity&, EventBus&, std::string_view, Entity_t);

	static constexpr CallSig value[] = { &call<Ts>... };
};

using kConnectScriptToEventDispatchTable =
	connect_script_to_event_dispatch_table<InspectorEventPanel::GuiEventTypeList>;

} // detail

void ConnectScriptToEvent(Entity& e, EventBus& bus, std::string_view selectedEvName,
						  std::string_view selectedScriptFn, Entity_t selectedRelevantEntity)
{
	int evIdx = GetCurrentEventNameIndex(selectedEvName);
	assert(evIdx >= 0 && evIdx < InspectorEventPanel::GuiEventTypeList::size);

	std::invoke(detail::kConnectScriptToEventDispatchTable::value[static_cast<size_t>(evIdx)],
				e, bus, selectedScriptFn, selectedRelevantEntity);
}

/** @} */

} // unnamed

/** @defgroup EditorEventCallbackBuilder @{ */

bool EditorEventCallbackBuilder::Draw(Entity& e, SceneFixture& fixture)
{
	if (!fixture.IsSystemRegistered<ScriptSystem>())
	{
		return false;
	}

	auto& scriptSys = fixture.GetSystem<ScriptSystem>();

	if (!BeginPropertyTable())
	{
		return false;
	}

	Property("event", [this] {
		DrawEventNames(selectedEventName_);
		return PropertyEditState::None;
	});

	if (selectedTableId_ != std::numeric_limits<ScriptTable::TableId>::max() &&
		!scriptSys.ContainsTable(selectedTableId_))
	{
		selectedTableId_ = std::numeric_limits<ScriptTable::TableId>::max();
		selectedTableFunction_.clear();
	}

	ScriptFilepathsContext filepathsCtx{
		.scriptTableMap = scriptSys.GetScriptTableMap(),
		.selectedTableId = selectedTableId_
	};

	Property("script", [&filepathsCtx] {
		return GuiEditProperty(filepathsCtx);
	});

	ScriptTableFunctionNamesContext funcNamesCtx{
		.scriptTableMap = scriptSys.GetScriptTableMap(),
		.selectedTableId = selectedTableId_,
		.selectedTableFunction = selectedTableFunction_
	};

	Property("function", [&funcNamesCtx] {
		return GuiEditProperty(funcNamesCtx);
	});

	Property("relevant entity", [&e, this] {
		DrawRelevantEntityList(e, selectedRelevantEntity_);
		return PropertyEditState::None;
	});

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	bool built = false;

	if (ImGui::Button("Done"))
	{
		if (CanAddCallback())
		{
			UpdateEntityScriptTable(e, scriptSys);

			ConnectScriptToEvent(e, fixture.GetEventBus(), selectedEventName_, 
								 selectedTableFunction_, selectedRelevantEntity_);

			UpdateEntityCallbackInfo(e, scriptSys);

			built = true;
		}
	}

	EndPropertyTable();

	return built;
}

void EditorEventCallbackBuilder::SetIsActiveImpl(bool val)
{
	if (IsActive() && !val)
	{
		ClearSelections();
	}
}

bool EditorEventCallbackBuilder::CanAddCallback()
{
	return !selectedEventName_.empty() &&
		   selectedTableId_ != std::numeric_limits<size_t>::max() &&
		   !selectedTableFunction_.empty();
}

void EditorEventCallbackBuilder::UpdateEntityScriptTable(Entity& e, const ScriptSystem& scriptSys)
{
	auto tableView = scriptSys.GetTableView(selectedTableId_);
	auto& tks = e.AddComponent<SignalTokenStorage>().signalTokens;

	auto& script = e.AddComponent<Script>();
	if (script.table.GetTableId() != selectedTableId_)
	{
		core::EraseIf(tks, [](const auto& tk) {
			return tk.type == EntityCallbackToken::Type::Script;
		});

		auto& callbackInfo = e.AddComponent<CallbackInfo>();
		callbackInfo.eventNames.clear();
		callbackInfo.scriptFileNames.clear();
		callbackInfo.tableFunctionNames.clear();
	}

	script.table = tableView;
}

void EditorEventCallbackBuilder::UpdateEntityCallbackInfo(Entity& e, const ScriptSystem& scriptSys)
{
	auto& callbackInfo = e.GetComponent<CallbackInfo>();
	callbackInfo.eventNames.emplace_back(selectedEventName_);
	callbackInfo.tableFunctionNames.emplace_back(selectedTableFunction_);

	const auto& selectedScriptFile = scriptSys.GetTableFilepath(selectedTableId_);
	assert(!selectedScriptFile.empty());

	auto path = fs::path(selectedScriptFile);
	assert(fs::exists(path));

	callbackInfo.scriptFileNames.emplace_back(path.filename().string());
}

void EditorEventCallbackBuilder::ClearSelections()
{
	selectedEventName_.clear();
	selectedTableId_ = std::numeric_limits<size_t>::max();
	selectedTableFunction_.clear();
	selectedRelevantEntity_ = kInvalidEntity;
}

/** @} */

} // ui

#endif