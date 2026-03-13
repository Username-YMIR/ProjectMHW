#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Weapons/Common/MHWeaponComboTypes.h"
#include "MHAnimNotifyState_ForesightPhaseWindow.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHAnimNotifyStateForesightPhaseWindow, Log, All);

UCLASS()
class PROJECTMHW_API UMHAnimNotifyState_ForesightPhaseWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    UPROPERTY(EditAnywhere, Category = "Combo")
    EMHLongSwordForesightPhase Phase = EMHLongSwordForesightPhase::None;
};
