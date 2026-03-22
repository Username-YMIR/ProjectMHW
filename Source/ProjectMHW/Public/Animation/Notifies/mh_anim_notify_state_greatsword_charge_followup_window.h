#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "mh_anim_notify_state_greatsword_charge_followup_window.generated.h"

UCLASS()
class PROJECTMHW_API UMHAnimNotifyState_GreatSwordChargeFollowUpWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GreatSword")
    FGameplayTag SourceMoveTag;
};
