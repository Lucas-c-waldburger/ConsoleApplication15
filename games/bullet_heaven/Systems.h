#pragma once
#include "../../ecs/Ecs.h"
#include "Weapon.h"

namespace game {

class WeaponSystem
{
public:
	void Update();

private:
	WeaponManager weaponManager_;
};

} // game