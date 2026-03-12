#pragma once

#include "CoreMinimal.h"
#include "Animation/mh_player_anim_instance_base.h"
#include "mh_player_anim_instance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHPlayerAnimInstance, Log, All);

/**
 * 플레이어 마스터 AnimInstance
 * - 공통 Locomotion 값은 Base에서 관리
 * - 무기별 현재 기술 MoveTag만 추가로 캐싱한다.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTMHW_API UMHPlayerAnimInstance : public UMHPlayerAnimInstanceBase
{
    GENERATED_BODY()

public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
