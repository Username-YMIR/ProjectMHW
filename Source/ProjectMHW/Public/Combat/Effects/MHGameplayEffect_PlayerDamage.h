#pragma once

#include "CoreMinimal.h"
#include "Combat/Effects/MHGameplayEffect_Damage.h"
#include "MHGameplayEffect_PlayerDamage.generated.h"

/**
 * 플레이어 전용 피격 처리용 Damage GameplayEffect
 * - 현재는 공통 DamageExecutionCalculation을 그대로 사용한다.
 * - 이후 플레이어 전용 피격/카운터/연출 정책을 이 계층에서 분리한다.
 */
UCLASS()
class PROJECTMHW_API UMHGameplayEffect_PlayerDamage : public UMHGameplayEffect_Damage
{
    GENERATED_BODY()

public:
    UMHGameplayEffect_PlayerDamage();
};
