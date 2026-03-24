#include "Animation/Notifies/mh_anim_notify_state_greatsword_early_transition_window.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UMHAnimNotifyState_GreatSwordEarlyTransitionWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp)
    {
        return;
    }

    if (AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->Notify_BeginGreatSwordEarlyTransitionWindow();
    }
}

void UMHAnimNotifyState_GreatSwordEarlyTransitionWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    if (AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->Notify_EndGreatSwordEarlyTransitionWindow();
    }
}
