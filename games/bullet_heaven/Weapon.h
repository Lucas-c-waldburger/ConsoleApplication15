#pragma once
#include <cstdint>
#include <limits>
#include "../../events/EventBus2.h"
#include "../../core/StableSOA.h"

class Entity;

namespace game {

using WeaponID = size_t;
using WeaponActionScriptID = size_t;

inline constexpr WeaponID 
kInvalidWeaponID = std::numeric_limits<WeaponID>::max();

using WeaponActionScript = fu2::unique_function<void(Entity&)>;

inline constexpr WeaponActionScriptID 
kInvalidWeaponActionScriptID = std::numeric_limits<WeaponActionScriptID>::max();

struct WeaponData
{
	std::string weaponName;
	std::string actionScriptName;
	std::string spriteSeriesName;
	int baseDamage = 0;
	float baseAttackSpeed = 0.0f;
	float baseSize = 0.0f;
};

using WeaponDataSOA = StableSOA<WeaponData,
	&WeaponData::weaponName,
	&WeaponData::actionScriptName,
	&WeaponData::spriteSeriesName,
	&WeaponData::baseDamage,
	&WeaponData::baseAttackSpeed,
	&WeaponData::baseSize
>;

class WeaponManager
{
public:
	WeaponID AddWeapon(WeaponData&& data, WeaponActionScript&& script);

private:
	WeaponDataSOA weaponData_;
	std::vector<WeaponActionScript> actionScripts_;
};

} // game