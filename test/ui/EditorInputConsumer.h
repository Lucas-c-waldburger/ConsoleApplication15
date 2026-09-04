#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../Fixtures.h"

namespace ui {

//template <MouseInputSource src, InputState st>
//struct MouseInputPair
//{
//	static constexpr MouseInputSource source = src;
//	static constexpr InputState state = st;
//
//	static constexpr bool Matches(MouseInputSource srcArg, InputState stArg)
//	{
//		return srcArg == source && stArg == state;
//	}
//};
//
//template <ImGuiKey src, InputState st>
//struct KeyInputPair
//{
//	static constexpr ImGuiKey source = src;
//	static constexpr InputState state = st;
//
//	static constexpr bool Matches(ImGuiKey srcArg, InputState stArg)
//	{
//		return srcArg == source && stArg == state;
//	}
//};
//
//namespace detail {
//template <typename T> struct is_mouse_input_pair : std::false_type {};
//template <MouseInputSource src, InputState st> 
//struct is_mouse_input_pair<MouseInputPair<src, st>> : std::true_type {};
//
//template <typename T> struct is_key_input_pair : std::false_type {};
//template <ImGuiKey src, InputState st>
//struct is_key_input_pair<KeyInputPair<src, st>> : std::true_type {};
//
//template <typename T> struct is_mouse_input_list : std::false_type {};
//template <template <typename...> class TList, typename...Ts>
//struct is_mouse_input_list<TList<Ts...>> : std::bool_constant<(is_mouse_input_pair<Ts>::value && ...)> {};
//
//template <typename T> struct is_key_input_list : std::false_type {};
//template <template <typename...> class TList, typename...Ts>
//struct is_key_input_list<TList<Ts...>> : std::bool_constant<(is_key_input_pair<Ts>::value && ...)> {};
//
//} // detail
//
//template <typename T>
//concept MouseInputList = detail::is_mouse_input_list<T>::value;
//
//template <typename T>
//concept KeyInputList = detail::is_key_input_list<T>::value;
//
//namespace detail {
//
//template <typename T> struct inputs_match_list;
//
//template <template <typename...> class TList, typename...Ts>
//struct inputs_match_list<TList<Ts...>>
//{
//	static constexpr bool call(ImGuiKey src, InputState st)
//		requires (is_key_input_pair<Ts>::value && ...)
//	{
//		return (Ts::Matches(src, st) || ...);
//	}
//
//	static constexpr bool call(MouseInputSource src, InputState st)
//		requires (is_mouse_input_pair<Ts>::value && ...)
//	{
//		return (Ts::Matches(src, st) || ...);
//	}
//};
//
//} // detail
//
//class IEditorInputConsumer
//{
//public:
//	virtual ~IEditorInputConsumer() = default;
//
//	virtual bool WantsInput(MouseInputSource, InputState) const { return false; }
//	virtual bool WantsInput(ImGuiKey, InputState) const { return false; }
//private:
//};
//
//template <typename...Pairs> requires (
//	(detail::is_mouse_input_pair<Pairs>::value || detail::is_key_input_pair<Pairs>::value) 
//&& ...)
//class EditorInputConsumer : public IEditorInputConsumer
//{
//public:
//	~EditorInputConsumer() override = default;
//
//	bool WantsInput(MouseInputSource src, InputState st) const override
//	{
//		return (Pairs::Matches(src, st))
//	}
//	bool WantsInput(ImGuiKey src, InputState st) const override
//	{
//		return detail::inputs_match_list<KeyL>::call(src, st);
//	}
//
//private:
//	template <typename Pair, typename Src, typename St>
//	static bool WantsInputImpl(Src src, St st)
//	{
//		if constexpr (detail::is_mouse_input_pair<Pair>::value)
//		{
//			return Pair::Matches(src, st);
//		}
//		else
//		{
//
//		}
//	}
//};

} // ui

#endif