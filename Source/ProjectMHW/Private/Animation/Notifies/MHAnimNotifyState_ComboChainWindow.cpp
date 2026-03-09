#include "Animation/Notifies/MHAnimNotifyState_ComboChainWindow.h"

#include "Character/Player/MHPlayerCharacter.h"

void UMHAnimNotifyState_ComboChainWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp)
    {
        return;
    }

    if (AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->Notify_BeginComboChainWindow();
    }
}

void UMHAnimNotifyState_ComboChainWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    if (AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner()))
    {
        Player->Notify_EndComboChainWindow();
    }
}
