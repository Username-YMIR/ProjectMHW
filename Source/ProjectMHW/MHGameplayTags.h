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
	// 무기 특수 입력 태그 (MB5)
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_WeaponSpecial); // 손승우 추가
	// 동시 입력 태그 (MB4)
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_AttackSimultaneous); // 손승우 추가
	// 조준/홀드 입력 태그 (C 홀드)
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_AimHold); // 손승우 추가

	
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
	
#pragma region Monster
	// State
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Monster_Unaware);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Monster_Alert);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Monster_Roaring);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Monster_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Monster_Dead);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Monster_Attacking);	// 근거리 패턴
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Monster_Charging);		// 원거리 돌진 
	
	// Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_DetectedBySight);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_AttackedFromUnaware);
	
	// Attack
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_Attack_Basic);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Monster_Attack_Charge);
	
	
#pragma endregion
	
#pragma region Data
	// Damage SetByCaller
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Physical);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Fire);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Water);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Thunder);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Ice);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Dragon);
	
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Weapon_AttackPower);
	PROJECTMHW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Weapon_Affinity);
#pragma endregion
	
	
	
}
