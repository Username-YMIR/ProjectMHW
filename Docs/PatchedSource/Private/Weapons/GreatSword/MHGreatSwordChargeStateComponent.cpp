#include "Weapons/GreatSword/MHGreatSwordChargeStateComponent.h"

#include "GameplayTags/MHGreatSwordGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHGreatSwordChargeStateComponent);

UMHGreatSwordChargeStateComponent::UMHGreatSwordChargeStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMHGreatSwordChargeStateComponent::ResetChargeState()
{
    CurrentChargeFamily = EMHGreatSwordChargeFamily::None;
    PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
    LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
    LastReleasedChargeFamily = EMHGreatSwordChargeFamily::None;
    CurrentChargeLevel = 0;
    LastReleasedChargeLevel = 0;
    bChargeReleaseReady = false;
    bStartedFromSheathedForwardInput = false;
}

void UMHGreatSwordChargeStateComponent::BeginCharge(const EMHGreatSwordChargeFamily InFamily, const bool bInStartedFromSheathedForward)
{
    CurrentChargeFamily = InFamily;
    CurrentChargeLevel = 0;
    bChargeReleaseReady = false;
    bStartedFromSheathedForwardInput = bInStartedFromSheathedForward;

    UE_LOG(LogMHGreatSwordChargeStateComponent, Verbose, TEXT("대검 차지 상태를 시작합니다. Family=%d DrawForward=%d"), static_cast<int32>(CurrentChargeFamily), bStartedFromSheathedForwardInput ? 1 : 0);
}

void UMHGreatSwordChargeStateComponent::NotifyChargeLevelReached(const int32 InChargeLevel)
{
    if (!IsCharging())
    {
        return;
    }

    CurrentChargeLevel = FMath::Clamp(InChargeLevel, 0, 2);
    bChargeReleaseReady = true;

    UE_LOG(LogMHGreatSwordChargeStateComponent, Verbose, TEXT("대검 차지 단계를 갱신합니다. Family=%d Level=%d"), static_cast<int32>(CurrentChargeFamily), CurrentChargeLevel);
}

void UMHGreatSwordChargeStateComponent::HandleTackleFromCurrentCharge()
{
    if (!IsCharging())
    {
        return;
    }

    LastTackleSourceChargeFamily = CurrentChargeFamily;
    PendingPostTackleChargeFamily = ResolveNextChargeFamilyAfterTackle(CurrentChargeFamily);
    CurrentChargeFamily = EMHGreatSwordChargeFamily::None;
    CurrentChargeLevel = 0;
    bChargeReleaseReady = false;
    bStartedFromSheathedForwardInput = false;

    UE_LOG(LogMHGreatSwordChargeStateComponent, Verbose, TEXT("대검 태클 후속 차지를 기록합니다. Source=%d Next=%d"), static_cast<int32>(LastTackleSourceChargeFamily), static_cast<int32>(PendingPostTackleChargeFamily));
}

EMHGreatSwordChargeFamily UMHGreatSwordChargeStateComponent::ConsumePostTackleChargeFamily()
{
    const EMHGreatSwordChargeFamily Result = PendingPostTackleChargeFamily;
    PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
    LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
    return Result;
}

void UMHGreatSwordChargeStateComponent::ClearPostTackleChargeState()
{
    PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
    LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
}

void UMHGreatSwordChargeStateComponent::SyncFromMirroredState(
    const EMHGreatSwordChargeFamily InCurrentChargeFamily,
    const int32 InCurrentChargeLevel,
    const EMHGreatSwordChargeFamily InPendingPostTackleChargeFamily,
    const EMHGreatSwordChargeFamily InLastTackleSourceChargeFamily,
    const EMHGreatSwordChargeFamily InLastReleasedChargeFamily,
    const int32 InLastReleasedChargeLevel,
    const bool bInChargeReleaseReady,
    const bool bInStartedFromSheathedForwardInput)
{
    CurrentChargeFamily = InCurrentChargeFamily;
    CurrentChargeLevel = FMath::Clamp(InCurrentChargeLevel, 0, 2);
    PendingPostTackleChargeFamily = InPendingPostTackleChargeFamily;
    LastTackleSourceChargeFamily = InLastTackleSourceChargeFamily;
    LastReleasedChargeFamily = InLastReleasedChargeFamily;
    LastReleasedChargeLevel = FMath::Clamp(InLastReleasedChargeLevel, 0, 2);
    bChargeReleaseReady = bInChargeReleaseReady;
    bStartedFromSheathedForwardInput = bInStartedFromSheathedForwardInput;
}

bool UMHGreatSwordChargeStateComponent::BuildReleaseDirective(FMHGreatSwordActionDirective& OutDirective)
{
    OutDirective.Reset();

    if (!IsCharging())
    {
        return false;
    }

    LastReleasedChargeFamily = CurrentChargeFamily;
    LastReleasedChargeLevel = CurrentChargeLevel;
    OutDirective.Type = EMHGreatSwordDirectiveType::PlayMove;

    if (bStartedFromSheathedForwardInput && !bChargeReleaseReady)
    {
        OutDirective.MoveTag = MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash;
    }
    else
    {
        OutDirective.MoveTag = ResolveChargeReleaseMoveTag(CurrentChargeFamily);
    }

    CurrentChargeFamily = EMHGreatSwordChargeFamily::None;
    CurrentChargeLevel = 0;
    bChargeReleaseReady = false;
    bStartedFromSheathedForwardInput = false;

    UE_LOG(LogMHGreatSwordChargeStateComponent, Verbose, TEXT("대검 차지 해제 지시를 만들었습니다. Move=%s Family=%d Level=%d"), *OutDirective.MoveTag.ToString(), static_cast<int32>(LastReleasedChargeFamily), LastReleasedChargeLevel);
    return OutDirective.IsValid();
}

bool UMHGreatSwordChargeStateComponent::BuildAutoReleaseDirective(FMHGreatSwordActionDirective& OutDirective)
{
    return BuildReleaseDirective(OutDirective);
}

EMHGreatSwordChargeFamily UMHGreatSwordChargeStateComponent::ResolveNextChargeFamilyAfterTackle(const EMHGreatSwordChargeFamily InCurrentFamily) const
{
    switch (InCurrentFamily)
    {
    case EMHGreatSwordChargeFamily::Charge:
        return EMHGreatSwordChargeFamily::Strong;
    case EMHGreatSwordChargeFamily::Strong:
    case EMHGreatSwordChargeFamily::TrueCharge:
        return EMHGreatSwordChargeFamily::TrueCharge;
    default:
        return EMHGreatSwordChargeFamily::Strong;
    }
}

FGameplayTag UMHGreatSwordChargeStateComponent::ResolveChargeReleaseMoveTag(const EMHGreatSwordChargeFamily InFamily) const
{
    switch (InFamily)
    {
    case EMHGreatSwordChargeFamily::Charge:
        return MHGreatSwordGameplayTags::Move_GS_ChargeSlash;
    case EMHGreatSwordChargeFamily::Strong:
        return MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash;
    case EMHGreatSwordChargeFamily::TrueCharge:
        return MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash;
    default:
        return FGameplayTag::EmptyTag;
    }
}
