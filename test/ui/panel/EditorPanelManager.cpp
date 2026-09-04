#include "EditorPanelManager.h"

#if IMGUI_ENABLED

namespace ui {
namespace detail {

namespace {

bool AssignIfError(Error& err, Result<Void>&& result)
{
	if (result.Success()) { return true; }

	err = std::move(result).GetError();

	return false;
}

template <typename> struct panel_dispatch_table;

template <template <typename...> class TList, typename...Ts>
struct panel_dispatch_table<TList<Ts...>>
{
	template <typename T>
	static void update_impl([[maybe_unused]] PanelType type, std::tuple<Ts...>& panelTup, 
							Entity e, SceneFixture& fixture)
	{
		assert(T::GetPanelType() == type);
		std::get<T>(panelTup).Update(e, fixture);
	}

	using UpdateSig = void(*)(PanelType, std::tuple<Ts...>&, Entity, SceneFixture&);

	template <typename T>
	static void clear_state_impl([[maybe_unused]] PanelType type, std::tuple<Ts...>& panelTup)
	{
		assert(T::GetPanelType() == type);
		std::get<T>(panelTup).ClearState();
	}

	using ClearStateSig = void(*)(PanelType, std::tuple<Ts...>&);

	static constexpr UpdateSig update[] = { &update_impl<Ts>... };
	static constexpr ClearStateSig clear_state[] = { &clear_state_impl<Ts>... };
};

} // unnamed

template <template <typename...> class TList, typename...Ts>
constexpr bool EditorPanelManagerTemplate<TList<Ts...>>::ValidPanelTypeIndex(PanelType type)
{
	const auto& idx = magic_enum::enum_index(type);

	return idx.has_value() && idx.value() >= 0 &&
		   idx.value() < std::tuple_size_v<PanelTuple>;
}

template <template <typename...> class TList, typename...Ts>
Result<Void> EditorPanelManagerTemplate<TList<Ts...>>::InitPanels(SceneFixture& fixture)
{
	Error err{};

	const bool success = (
		(AssignIfError(err, (std::get<Ts>(panels_).Init(fixture)))
	) && ...);
	if (!success)
	{
		return err;
	}

	return kVoid;
}

template <template <typename...> class TList, typename...Ts>
void EditorPanelManagerTemplate<TList<Ts...>>::TearDownPanels()
{
	std::apply([](auto&&...panels) {
		((panels.TearDown()), ...);
	}, panels_);
}

template <template <typename...> class TList, typename...Ts>
bool EditorPanelManagerTemplate<TList<Ts...>>::IsPanelActive(PanelType type)
{
	if (!ValidPanelTypeIndex(type))
	{
		return false;
	}

	return active_.test(magic_enum::enum_index(type).value());
}

template <template <typename...> class TList, typename...Ts>
void EditorPanelManagerTemplate<TList<Ts...>>::SetPanelActive(PanelType type, bool val)
{
	if (!ValidPanelTypeIndex(type))
	{
		return;
	}
	
	const size_t panelIdx = magic_enum::enum_index(type).value();

	const bool wasActive = active_.test(panelIdx);
	if (wasActive == val)
	{
		return;
	}
	
	if (!val)
	{
		panel_dispatch_table<PanelTuple>::clear_state[panelIdx](type, panels_);
	}
	
	active_.set(panelIdx, val);
}

template <template <typename...> class TList, typename...Ts>
void EditorPanelManagerTemplate<TList<Ts...>>::UpdatePanel(PanelType type, Entity e, SceneFixture& fixture)
{
	if (!IsPanelActive(type))
	{
		return;
	}

	const size_t panelIdx = magic_enum::enum_index(type).value();

	panel_dispatch_table<PanelTuple>::update[panelIdx](type, panels_, e, fixture);
}

template class EditorPanelManagerTemplate<EditorPanelTypeList>;

} // detail
} // ui

#endif