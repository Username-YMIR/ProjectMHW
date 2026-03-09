#include "MHGameplayTags.h"

namespace MHGameplayTags
{
	//Input Tags
	UE_DEFINE_GAMEPLAY_TAG(Input_Move, "Input.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Look, "Input.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Sprint, "Input.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Input_Dodge, "Input.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Input_Interact, "Input.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Input_AttackPrimary, "Input.Attack.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Input_AttackSecondary, "Input.Attack.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Input_WeaponSpecial, "Input.Weapon.Special"); // 손승우 추가
	UE_DEFINE_GAMEPLAY_TAG(Input_AttackSimultaneous, "Input.Attack.Simultaneous"); // 손승우 추가
	UE_DEFINE_GAMEPLAY_TAG(Input_AimHold, "Input.Aim.Hold"); // 손승우 추가

	
	//Element Tags (속성 태그)
	UE_DEFINE_GAMEPLAY_TAG(Element,        "Element");
	UE_DEFINE_GAMEPLAY_TAG(Element_Fire,   "Element.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Element_Water,  "Element.Water");
	UE_DEFINE_GAMEPLAY_TAG(Element_Thunder,"Element.Thunder");
	UE_DEFINE_GAMEPLAY_TAG(Element_Ice,    "Element.Ice");
	UE_DEFINE_GAMEPLAY_TAG(Element_Dragon, "Element.Dragon");

	
	
#pragma region ItemTags
	// Root
	UE_DEFINE_GAMEPLAY_TAG(Item, "Item");

	// Item Categories
	UE_DEFINE_GAMEPLAY_TAG(Item_Equipment, "Item.Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Item_Equipment_Weapon, "Item.Equipment.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Item_Equipment_Armor, "Item.Equipment.Armor");

	UE_DEFINE_GAMEPLAY_TAG(Item_Common, "Item.Common");
	UE_DEFINE_GAMEPLAY_TAG(Item_Common_Consumable, "Item.Common.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Common_Material, "Item.Common.Material");


	// Weapon Types
	UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_GreatSword, "Item.Equipment.Weapon.GreatSword");
	UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_LongSword, "Item.Equipment.Weapon.LongSword");


	// Armor Slots
	UE_DEFINE_GAMEPLAY_TAG(Item_Armor_Head, "Item.Equipment.Armor.Head");
	UE_DEFINE_GAMEPLAY_TAG(Item_Armor_Chest, "Item.Equipment.Armor.Chest");
	UE_DEFINE_GAMEPLAY_TAG(Item_Armor_Arms, "Item.Equipment.Armor.Arms");
	UE_DEFINE_GAMEPLAY_TAG(Item_Armor_Waist, "Item.Equipment.Armor.Waist");
	UE_DEFINE_GAMEPLAY_TAG(Item_Armor_Legs, "Item.Equipment.Armor.Legs");
	UE_DEFINE_GAMEPLAY_TAG(Item_Armor_Charm, "Item.Equipment.Armor.Charm");


	// Consumables
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable_Recovery, "Item.Common.Consumable.Recovery");
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable_Buff, "Item.Common.Consumable.Buff");
#pragma endregion
}