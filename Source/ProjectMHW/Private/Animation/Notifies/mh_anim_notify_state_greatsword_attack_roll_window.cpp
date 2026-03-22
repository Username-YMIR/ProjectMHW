#include "Animation/Notifies/mh_anim_notify_state_greatsword_attack_roll_window.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UMHAnimNotifyState_GreatSwordAttackRollWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp)
    {
        return;
    }

    AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner());
    if (!Player)
    {
        return;
    }

    Player->Notify_BeginGreatSwordAttackRollWindow();
}

void UMHAnimNotifyState_GreatSwordAttackRollWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner());
    if (!Player)
    {
        return;
    }

    Player->Notify_EndGreatSwordAttackRollWindow();
}
