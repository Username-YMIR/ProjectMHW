#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "mh_anim_notify_state_early_transition_window.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHAnimNotifyStateEarlyTransitionWindow, Log, All);

UCLASS()
class PROJECTMHW_API UMHAnimNotifyState_EarlyTransitionWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    UPROPERTY(EditAnywhere, Category = "Combo")
    FName WindowName = NAME_None;
};
