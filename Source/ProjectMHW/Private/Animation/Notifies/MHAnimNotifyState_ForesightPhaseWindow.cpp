#include "Animation/Notifies/MHAnimNotifyState_ForesightPhaseWindow.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Items/Instance/MHLongSwordInstance.h"
#include "Weapons/LongSword/MHLongSwordComboComponent.h"

DEFINE_LOG_CATEGORY(LogMHAnimNotifyStateForesightPhaseWindow);

void UMHAnimNotifyState_ForesightPhaseWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp)
    {
        return;
    }

    const AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner());
    if (!Player)
    {
        return;
    }

    const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(Player->GetEquippedWeapon());
    if (!LongSword)
    {
        return;
    }

    UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp)
    {
        return;
    }

    ComboComp->SetForesightPhase(Phase);
    UE_LOG(LogMHAnimNotifyStateForesightPhaseWindow, Verbose, TEXT("간파 입력 구간 시작. Owner=%s Phase=%d"), *Player->GetName(), static_cast<int32>(Phase));
}

void UMHAnimNotifyState_ForesightPhaseWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    const AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(MeshComp->GetOwner());
    if (!Player)
    {
        return;
    }

    const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(Player->GetEquippedWeapon());
    if (!LongSword)
    {
        return;
    }

    UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp)
    {
        return;
    }

    if (ComboComp->GetForesightPhase() == Phase)
    {
        ComboComp->SetForesightPhase(EMHLongSwordForesightPhase::None);
        UE_LOG(LogMHAnimNotifyStateForesightPhaseWindow, Verbose, TEXT("간파 입력 구간 종료. Owner=%s Phase=%d"), *Player->GetName(), static_cast<int32>(Phase));
    }
}
