#include "Animation/Notifies/mh_anim_notify_state_early_transition_window.h"

#include "Character/Player/MHPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogMHAnimNotifyStateEarlyTransitionWindow);

void UMHAnimNotifyState_EarlyTransitionWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
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

    UE_LOG(LogMHAnimNotifyStateEarlyTransitionWindow, Verbose, TEXT("조기 전환 윈도우 시작. Owner=%s Window=%s"), *Player->GetName(), *WindowName.ToString());
    Player->Notify_BeginEarlyTransitionWindow();
}

void UMHAnimNotifyState_EarlyTransitionWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

    UE_LOG(LogMHAnimNotifyStateEarlyTransitionWindow, Verbose, TEXT("조기 전환 윈도우 종료. Owner=%s Window=%s"), *Player->GetName(), *WindowName.ToString());
    Player->Notify_EndEarlyTransitionWindow();
}
