#include "Animation/Notifies/mh_anim_notify_greatsword_charge_auto_release.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UMHAnimNotify_GreatSwordChargeAutoRelease::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner());
    if (!Player)
    {
        return;
    }

    Player->Notify_GreatSwordChargeAutoRelease();
}
