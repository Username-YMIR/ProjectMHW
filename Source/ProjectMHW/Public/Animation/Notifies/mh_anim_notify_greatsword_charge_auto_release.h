#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "mh_anim_notify_greatsword_charge_auto_release.generated.h"

UCLASS()
class PROJECTMHW_API UMHAnimNotify_GreatSwordChargeAutoRelease : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
};
