#pragma once
#include <iostream>

// TODO: consider making enum class
enum InputState : uint8_t
{
	None = 0, 
	Pressed, 
	Released, 
	Held 
};

inline std::ostream& operator<<(std::ostream& os, const InputState& st)
{
	os << (st == InputState::Pressed)  ? "Pressed" :
		  (st == InputState::Released) ? "Released" :
		  (st == InputState::Held)     ? "Held" : "None";

	return os;
}