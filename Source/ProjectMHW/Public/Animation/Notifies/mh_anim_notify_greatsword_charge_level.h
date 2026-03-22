#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "mh_anim_notify_greatsword_charge_level.generated.h"

UCLASS()
class PROJECTMHW_API UMHAnimNotify_GreatSwordChargeLevel : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

protected:
    // 차징 몽타주 내부 타이밍에서 확정할 단계 값
    UPROPERTY(EditAnywhere, Category = "GreatSword")
    int32 ChargeLevel = 0;
};
