#include "GameplayTags/MHCombatStateGameplayTags.h"

//손승우 추가: 무기 타입 / 납도 상태 / 전투 상태 전용 네이티브 게임플레이 태그 정의

namespace MHCombatStateGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(WeaponType_LongSword, "WeaponType.LongSword");
	UE_DEFINE_GAMEPLAY_TAG(WeaponType_ChargeBlade, "WeaponType.ChargeBlade");
	UE_DEFINE_GAMEPLAY_TAG(WeaponSheath_Sheathed, "WeaponSheath.Sheathed");
	UE_DEFINE_GAMEPLAY_TAG(WeaponSheath_Unsheathing, "WeaponSheath.Unsheathing");
	UE_DEFINE_GAMEPLAY_TAG(WeaponSheath_Unsheathed, "WeaponSheath.Unsheathed");
	UE_DEFINE_GAMEPLAY_TAG(WeaponSheath_Sheathing, "WeaponSheath.Sheathing");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_None, "CombatState.None");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_Draw, "CombatState.Draw");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_Sheathe, "CombatState.Sheathe");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_Dodge, "CombatState.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_Attack, "CombatState.Attack");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_SpiritAttack, "CombatState.SpiritAttack");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_Counter, "CombatState.Counter");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_SpecialSheathe, "CombatState.SpecialSheathe");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_Helmbreaker, "CombatState.Helmbreaker");
	UE_DEFINE_GAMEPLAY_TAG(CombatState_Clutch, "CombatState.Clutch");
}
