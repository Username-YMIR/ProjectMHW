#include "Animation/Notifies/MHAnimNotifyState_LongSwordCounterWindow.h"

#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY(LogMHANSLongSwordCounterWindow)

FString UMHAnimNotifyState_LongSwordCounterWindow::GetNotifyName_Implementation() const
{
    return TEXT("MH LongSword Counter Window");
}

void UMHAnimNotifyState_LongSwordCounterWindow::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (AMHPlayerCharacter* PlayerCharacter = ResolvePlayerCharacter(MeshComp))
    {
        PlayerCharacter->Notify_BeginLongSwordCounterWindow(CounterWindowType);
        UE_LOG(LogMHANSLongSwordCounterWindow, Verbose, TEXT("%s : Begin counter window. Type=%d"),
            *PlayerCharacter->GetName(),
            static_cast<int32>(CounterWindowType));
    }
}

void UMHAnimNotifyState_LongSwordCounterWindow::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (AMHPlayerCharacter* PlayerCharacter = ResolvePlayerCharacter(MeshComp))
    {
        PlayerCharacter->Notify_EndLongSwordCounterWindow(CounterWindowType);
        UE_LOG(LogMHANSLongSwordCounterWindow, Verbose, TEXT("%s : End counter window. Type=%d"),
            *PlayerCharacter->GetName(),
            static_cast<int32>(CounterWindowType));
    }
}

AMHPlayerCharacter* UMHAnimNotifyState_LongSwordCounterWindow::ResolvePlayerCharacter(USkeletalMeshComponent* MeshComp) const
{
    return MeshComp ? Cast<AMHPlayerCharacter>(MeshComp->GetOwner()) : nullptr;
}
