#pragma once

#include "NativeGameplayTags.h"

namespace MHGameplayTags
{
	// 이동 입력 태그
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Move);
	// 시점 입력 태그
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Look);
	// 스프린트 입력 태그
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Sprint);
	// 회피 입력 태그
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Dodge);
	// 상호작용 입력 태그
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Interact);
	// 기본 공격 입력 태그
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_AttackPrimary);
	// 보조 공격 입력 태그
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_AttackSecondary);

	
	//Element Tags (속성 태그)
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Fire);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Water);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Thunder);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Ice);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Element_Dragon);
	

	
#pragma region ItemTags
	// Root
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item);

	// Item Categories
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Common);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Common_Consumable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Common_Material);

	// Weapon Types
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_GreatSword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_LongSword);

	// Armor Slots
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Head);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Chest);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Arms);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Waist);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Legs);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Charm);

	// Consumables
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Common_Consumable_Recovery);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Common_Consumable_Buff);
#pragma endregion
}
