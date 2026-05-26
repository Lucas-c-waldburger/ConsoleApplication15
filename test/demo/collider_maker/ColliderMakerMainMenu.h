#pragma once
#include "ColliderMakerMenu.h"
#include "NavigationStack.h"

namespace test {

class ColliderMakerLayer;

class ColliderMakerMainMenu : public ColliderMakerMenu
{
public:
	ColliderMakerMainMenu() : ColliderMakerMenu(MenuType::Main, "") {}

	~ColliderMakerMainMenu() override = default;

	void Draw(NavigationStack& navStack) override;

private:
	std::set<std::string> layerIdentifiers_;
	std::string selectedLayer_;
};

} // test