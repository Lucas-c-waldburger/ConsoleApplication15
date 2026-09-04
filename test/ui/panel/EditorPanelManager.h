#pragma once
#include "EditorEntityPanel.h"
#include "EditorComponentPanel.h"
#include "EditorSystemPanel.h"
#include "EditorEventPanel.h"
#include <magic_enum/magic_enum.hpp>

#if IMGUI_ENABLED

namespace ui {

using EditorPanelTypeList = TypeList<
	EditorEntityPanel,
	EditorComponentPanel,
	EditorSystemPanel,
	EditorEventPanel
>;

namespace detail {

template <typename T, typename List>
struct inserted_sorted_panel;

template <typename T>
struct inserted_sorted_panel<T, TypeList<>>
{
    using type = TypeList<T>;
};

template <typename T, typename U, typename... Us>
struct inserted_sorted_panel<T, TypeList<U, Us...>>
{
    using type = std::conditional_t <
        static_cast<int>(T::GetPanelType()) <
        static_cast<int>(U::GetPanelType()),

        TypeList<T, U, Us...>,

        prepend_type_t<
            U,
            typename inserted_sorted_panel<T, TypeList<Us...>>::type
        >
    >;
};

template <typename List, typename T>
struct sort_insert;

template <typename... Ts, typename T>
struct sort_insert<TypeList<Ts...>, T>
{
    using type = typename inserted_sorted_panel<T, TypeList<Ts...>>::type;
};

template <typename... Ts>
struct sort_panels;

template <>
struct sort_panels<>
{
    using type = TypeList<>;
};

template <typename T, typename... Ts>
struct sort_panels<T, Ts...>
{
    using type = typename sort_insert<
        typename sort_panels<Ts...>::type,
        T
    >::type;
};

template <typename... Ts>
using sort_panels_t = typename sort_panels<Ts...>::type;

template <typename> class EditorPanelManagerTemplate;

template <template <typename...> class TList, typename...Ts>
class EditorPanelManagerTemplate<TList<Ts...>>
{
public:
	using PanelTuple = as_tuple_t<sort_panels_t<Ts...>>;
	using PanelActiveBitset = std::bitset<magic_enum::enum_count<PanelType>()>;

	Result<Void> InitPanels(SceneFixture& fixture);
	void TearDownPanels();

	bool IsPanelActive(PanelType type);
	void SetPanelActive(PanelType type, bool val);
	void UpdatePanel(PanelType type, Entity e, SceneFixture& fixture);

    template <typename T> requires (std::same_as<T, Ts> || ...)
    T& GetPanel() { return std::get<T>(panels_); }

private:
	static constexpr bool ValidPanelTypeIndex(PanelType type);

	PanelTuple panels_;
	PanelActiveBitset active_;
};

extern template class EditorPanelManagerTemplate<EditorPanelTypeList>;

} // detail

using EditorPanelManager = detail::EditorPanelManagerTemplate<EditorPanelTypeList>;

} // ui

#endif