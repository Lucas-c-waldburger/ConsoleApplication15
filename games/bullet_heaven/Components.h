#pragma once
#include "../../components/BaseComponent.h"
#include "Weapon.h"
#include "Item.h"

namespace game {

//template <typename Derived, typename ValueType>
//struct ValueComponent;
//
//template <typename Derived, typename ValueType>
//	requires std::is_default_constructible_v<ValueType>
//struct ValueComponent<Derived, ValueType> : BaseComponent<Derived>
//{
//	ValueType value{};
//};
//
//
//template <typename Derived, typename BaseValueType = float>
//	requires std::is_default_constructible_v<BaseValueType>
//struct StatComponent : BaseComponent<Derived>
//{
//	BaseValueType base{};
//	float scale = 1.0f;
//};
//
//// STATS
//struct MovementSpeed : StatComponent<MovementSpeed> {};
//struct JumpHeight : StatComponent<JumpHeight> {};
//struct ExtraJumps : ValueComponent<ExtraJumps, int> {};
//struct Size : StatComponent<Size> {};
//struct AttackSpeed : StatComponent<AttackSpeed> {};
//
//
//// STATE
//struct PlayerState : BaseComponent<PlayerState>
//{
//	enum 
//	{
//		Grounded,
//		Airborne
//	} state;
//};
//
//struct PlayerWeapons : BaseComponent<PlayerWeapons>
//{
//	std::array<WeaponID, 4> weaponIds = {
//		kInvalidWeaponID, kInvalidWeaponID, 
//		kInvalidWeaponID, kInvalidWeaponID 
//	};
//	size_t count = 0;
//};
//
//struct PlayerItems : BaseComponent<PlayerItems>
//{
//	std::array<ItemID, 4> itemIds = {
//		kInvalidItemID, kInvalidItemID,
//		kInvalidItemID, kInvalidItemID
//	};
//	size_t count = 0;
//};

} // game