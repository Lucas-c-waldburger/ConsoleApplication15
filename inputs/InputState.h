#pragma once
#include <iostream>

// TODO: consider making enum class
enum InputState : uint8_t
{
	None = 0, 
	Pressed = 1 << 0, 
	Released = 1 << 1, 
	Held = 1 << 2,
	AnyInput = Pressed | Released | Held
};

inline constexpr InputState operator|(InputState lhs, InputState rhs)
{
	return static_cast<InputState>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

inline constexpr InputState operator&(InputState lhs, InputState rhs)
{
	return static_cast<InputState>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

inline std::ostream& operator<<(std::ostream& os, const InputState& st)
{
	os << (st == InputState::Pressed)  ? "Pressed" :
		  (st == InputState::Released) ? "Released" :
		  (st == InputState::Held)     ? "Held" : "None";

	return os;
}