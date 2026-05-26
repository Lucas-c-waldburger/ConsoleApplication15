#pragma once

#include "ColliderMakerMenu.h"

namespace test {

class ColliderMakerLayer : public ColliderMakerMenu
{
public:
	explicit ColliderMakerLayer(std::string_view ident) : 
		ColliderMakerMenu(MenuType::Layer, ident) {}

	~ColliderMakerLayer() override = default;

private:
};


} // test