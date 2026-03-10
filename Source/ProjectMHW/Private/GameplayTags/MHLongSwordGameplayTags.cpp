#include "GameplayTags/MHLongSwordGameplayTags.h"

//손승우 수정: 롱소드 Move 태그를 입력 패턴 설계 기준에 맞게 정렬

namespace MHLongSwordGameplayTags
{
	// ===== Draw / Entry =====
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_DrawOnly, "Move.LS.DrawOnly");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_DrawAdvancingSlash, "Move.LS.DrawAdvancingSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_DrawSpiritSlash1, "Move.LS.DrawSpiritSlash1");
	// ===== End Draw / Entry =====

	// ===== Normal =====
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_AdvancingSlash, "Move.LS.AdvancingSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_VerticalSlash, "Move.LS.VerticalSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_Thrust, "Move.LS.Thrust");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_RisingSlash, "Move.LS.RisingSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_DownwardSlash, "Move.LS.DownwardSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_FadeSlash, "Move.LS.FadeSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_LateralFadeSlash, "Move.LS.LateralFadeSlash");
	// ===== End Normal =====

	// ===== Spirit =====
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpiritSlash1, "Move.LS.SpiritSlash1");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpiritSlash2, "Move.LS.SpiritSlash2");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpiritSlash3, "Move.LS.SpiritSlash3");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpiritRoundslash, "Move.LS.SpiritRoundslash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpiritAdvancingSlash, "Move.LS.SpiritAdvancingSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpiritThrust, "Move.LS.SpiritThrust");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpiritHelmbreaker, "Move.LS.SpiritHelmbreaker");
	// ===== End Spirit =====

	// ===== Special / Counter =====
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_SpecialSheathe, "Move.LS.SpecialSheathe");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_IaiSlash, "Move.LS.IaiSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_IaiSpiritSlash, "Move.LS.IaiSpiritSlash");
	UE_DEFINE_GAMEPLAY_TAG(Move_LS_ForesightSlash, "Move.LS.ForesightSlash");
	// ===== End Special / Counter =====
}