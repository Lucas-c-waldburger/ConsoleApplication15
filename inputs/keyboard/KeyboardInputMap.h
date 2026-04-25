#pragma once
#include "KeyboardInputField.h"
#include "../InputMap.h"

using KeyboardInputMap = InputMap<KeyboardInputSource, KeyboardInputField>;

namespace detail {

using Key = KeyboardInputSource;

constexpr char GetKeyChar(KeyboardInputSource key)
{
	static constexpr char offset = static_cast<char>(97 - 4);

	if (static_cast<size_t>(key) >= 4 && static_cast<size_t>(key) <= 29)
	{
		return static_cast<char>(key) + offset;
	}

	switch (key)
	{
	case Key::Num1: return '1';
	case Key::Num2: return '2';
	case Key::Num3: return '3';
	case Key::Num4: return '4';
	case Key::Num5: return '5';
	case Key::Num6: return '6';
	case Key::Num7: return '7';
	case Key::Num8: return '8';
	case Key::Num9: return '9';
	case Key::Num0: return '0';
	case Key::Minus: return '-';
	case Key::Equals: return '=';
	case Key::LeftBracket: return '-';
	case Key::RightBracket: return ']';
	case Key::Backslash: return '\\';
	case Key::Semicolon: return ';';
	case Key::Apostrophe: return '\'';
	case Key::Grave: return '`';
	case Key::Comma: return ',';
	case Key::Period: return '.';
	case Key::Slash: return '/';
	default: return '\0';
	}
}

template <>
struct make_input_map_impl<KeyboardInputMap>
{
	static constexpr KeyboardInputMap call()
	{
		KeyboardInputMap inputMap{};

		for (size_t i = enum_start_v<KeyboardInputSource>; 
			 i < enum_size_v<KeyboardInputSource>; ++i)
		{
			auto src = static_cast<KeyboardInputSource>(i);

			inputMap[src].source = src;
			inputMap[src].state = InputState::None;
			inputMap[src].value = GetKeyChar(src);			
		}

		return inputMap;
	}
};

} // detail