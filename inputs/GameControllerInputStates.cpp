#include "GameControllerInputStates.h"

std::ostream& operator<<(std::ostream& os, const AxisInputState& inp)
{
	os << "XY: { " << inp.value.x << ", " << inp.value.y << " }\nTimestamp: "
		<< inp.timestamp << "\nState: " << inp.state << "\nStateDuration: "
		<< inp.stateDuration << "\n\n";

	return os;
}
std::ostream& operator<<(std::ostream& os, const ButtonInputState& inp)
{
	os << "Button: " << SDL_GameControllerGetStringForButton(inp.button) << "\nTimestamp: "
		<< inp.timestamp << "\nState: " << inp.state << "\nStateDuration: "
		<< inp.stateDuration << "\n\n";

	return os;
}