#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "mh_linked_anim_layer_base.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHLinkedAnimLayerBase, Log, All);

class UMHPlayerAnimInstance;

/**
 * 무기별 Linked Anim Layer 공통 베이스
 * - 마스터 AnimInstance를 안전하게 가져오는 진입점만 제공한다.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTMHW_API UMHLinkedAnimLayerBase : public UAnimInstance
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Anim")
    UMHPlayerAnimInstance* GetMHPlayerAnimInstance() const;
};
