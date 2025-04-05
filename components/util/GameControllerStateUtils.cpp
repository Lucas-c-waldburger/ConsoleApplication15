#include "GameControllerStateUtils.h"


std::ostream& operator<<(std::ostream& os, const GameControllerState::State& st)
{
	using ST = GameControllerState::State;
	os << (st == ST::Pressed)  ? "Pressed" : 
		  (st == ST::Released) ? "Released" :
		  (st == ST::Held)     ? "Held" : "None";

	return os;
}