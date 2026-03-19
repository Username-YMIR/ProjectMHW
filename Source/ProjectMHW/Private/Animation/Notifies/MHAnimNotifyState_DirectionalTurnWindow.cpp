#include "Animation/Notifies/MHAnimNotifyState_DirectionalTurnWindow.h"

#include "Character/Player/MHPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogMHAnimNotifyStateDirectionalTurnWindow);

void UMHAnimNotifyState_DirectionalTurnWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
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

    UE_LOG(LogMHAnimNotifyStateDirectionalTurnWindow, Verbose,
        TEXT("회전 조정 윈도우 시작. Owner=%s MaxYawDelta=%.2f InterpSpeed=%.2f"),
        *Player->GetName(),
        MaxYawDeltaDegrees,
        RotationInterpSpeed);

    Player->Notify_BeginDirectionalTurnWindow(MaxYawDeltaDegrees, RotationInterpSpeed);
}

void UMHAnimNotifyState_DirectionalTurnWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

    UE_LOG(LogMHAnimNotifyStateDirectionalTurnWindow, Verbose,
        TEXT("회전 조정 윈도우 종료. Owner=%s"),
        *Player->GetName());

    Player->Notify_EndDirectionalTurnWindow();
}
