#include "Animation/Notifies/mh_anim_notify_state_greatsword_charge_followup_window.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UMHAnimNotifyState_GreatSwordChargeFollowUpWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp)
    {
        return;
    }

    if (AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->Notify_BeginGreatSwordChargeFollowUpWindow(SourceMoveTag);
    }
}

void UMHAnimNotifyState_GreatSwordChargeFollowUpWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    if (AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->Notify_EndGreatSwordChargeFollowUpWindow();
    }
}
