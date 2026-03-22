#include "Weapons/GreatSword/MHGreatSwordActionComponent.h"

#include "Animation/AnimMontage.h"
#include "Combat/Data/MHAttackMetaTypes.h"
#include "Combat/Data/MHCombatDataLibrary.h"
#include "GameplayTags/MHGreatSwordGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHGreatSwordActionComponent);

UMHGreatSwordActionComponent::UMHGreatSwordActionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UMHGreatSwordActionComponent::HandlePrimaryPressed(const bool bInForwardInput, const bool bInSheathed)
{
    if (IsCharging() || IsGuarding())
    {
        return false;
    }

    if (bInSheathed)
    {
        if (bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::Charge, true);
            return true;
        }

        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_DrawOnly, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        const EMHGreatSwordChargeFamily NextFamily = PendingPostTackleChargeFamily != EMHGreatSwordChargeFamily::None
            ? PendingPostTackleChargeFamily
            : EMHGreatSwordChargeFamily::Strong;

        PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
        BeginCharging(NextFamily, false);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash)
    {
        if (bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::Strong, false);
        }
        else
        {
            QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        }
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash)
    {
        if (bChargeFollowUpWindowOpen && ChargeFollowUpSourceMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash && bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::Strong, false);
        }
        else
        {
            BeginCharging(EMHGreatSwordChargeFamily::Charge, false);
        }
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash)
    {
        if (bChargeFollowUpWindowOpen && ChargeFollowUpSourceMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash && bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::TrueCharge, false);
        }
        else
        {
            BeginCharging(EMHGreatSwordChargeFamily::Charge, false);
        }
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash)
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash)
    {
        BeginCharging(EMHGreatSwordChargeFamily::Charge, false);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_RisingSlash)
    {
        BeginCharging(EMHGreatSwordChargeFamily::Charge, false);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_WideSlash)
    {
        BeginCharging(EMHGreatSwordChargeFamily::Charge, false);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    BeginCharging(EMHGreatSwordChargeFamily::Charge, false);
    return true;
}

bool UMHGreatSwordActionComponent::HandlePrimaryReleased()
{
    if (!IsCharging())
    {
        return false;
    }

    return FinalizeCurrentChargeRelease();
}

bool UMHGreatSwordActionComponent::HandleSecondaryPressed()
{
    if (IsCharging())
    {
        PendingPostTackleChargeFamily = ResolveNextChargeFamilyAfterTackleFromCharge(ChargeFamily);
        ResetChargeContext();
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsGuarding())
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_WideSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_RisingSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_OverheadSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash)
    {
        return false;
    }

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        return false;
    }

    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_OverheadSlash, EMHGreatSwordActionState::Acting);
    return true;
}

bool UMHGreatSwordActionComponent::HandleWeaponSpecialPressed(const bool bInSheathed)
{
    if (IsCharging())
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsGuarding())
    {
        return false;
    }

    bGuardHeld = true;
    QueuePendingMove(
        bInSheathed ? MHGreatSwordGameplayTags::Move_GS_DrawGuard : MHGreatSwordGameplayTags::Move_GS_Guard,
        EMHGreatSwordActionState::Guarding
    );
    return true;
}

bool UMHGreatSwordActionComponent::HandleWeaponSpecialReleased()
{
    if (!IsGuarding())
    {
        bGuardHeld = false;
        return false;
    }

    bGuardHeld = false;
    ActionState = EMHGreatSwordActionState::Acting;
    return true;
}

bool UMHGreatSwordActionComponent::HandleSimultaneousPressed()
{
    if (IsCharging() || IsGuarding())
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash)
    {
        return false;
    }

    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_RisingSlash, EMHGreatSwordActionState::Acting);
    return true;
}

FGameplayTag UMHGreatSwordActionComponent::ConsumePendingMoveTag()
{
    const FGameplayTag ResultTag = PendingMoveTag;
    PendingMoveTag = FGameplayTag();
    return ResultTag;
}

float UMHGreatSwordActionComponent::ResolveChargeDamageScaleForMove(const FGameplayTag& InMoveTag) const
{
    auto ResolveScale = [this](const FVector& InScaleVector) -> float
    {
        switch (FMath::Clamp(LastReleasedChargeLevel, 0, 2))
        {
        case 0:
            return InScaleVector.X;
        case 1:
            return InScaleVector.Y;
        default:
            return InScaleVector.Z;
        }
    };

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash && LastReleasedChargeFamily == EMHGreatSwordChargeFamily::Charge)
    {
        return ResolveScale(ChargeSlashLevelDamageScale);
    }

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash && LastReleasedChargeFamily == EMHGreatSwordChargeFamily::Strong)
    {
        return ResolveScale(StrongChargeSlashLevelDamageScale);
    }

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash && LastReleasedChargeFamily == EMHGreatSwordChargeFamily::TrueCharge)
    {
        return ResolveScale(TrueChargeSlashLevelDamageScale);
    }

    return 1.0f;
}

void UMHGreatSwordActionComponent::CommitExecutedMove(const FGameplayTag& InMoveTag)
{
    LastCommittedMoveTag = InMoveTag;
    PendingMoveTag = FGameplayTag();
    ActiveUtilityMoveTag = FGameplayTag();
    ResetChargeFollowUpState();
    ActionState = EMHGreatSwordActionState::Acting;
}

void UMHGreatSwordActionComponent::NotifyUtilityMoveStarted(const FGameplayTag& InMoveTag)
{
    ActiveUtilityMoveTag = InMoveTag;
    PendingMoveTag = FGameplayTag();

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_Guard || InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawGuard)
    {
        LastCommittedMoveTag = FGameplayTag();
        ResetChargeFollowUpState();
        ActionState = EMHGreatSwordActionState::Guarding;
        return;
    }

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawDefaultCharge
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlashCharging
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlashCharging
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlashCharging)
    {
        ActionState = EMHGreatSwordActionState::Charging;
        return;
    }

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawOnly
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_Sheathe
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollFront
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollBack
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollLeft
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollRight)
    {
        LastCommittedMoveTag = FGameplayTag();
        ResetChargeFollowUpState();
        ActionState = EMHGreatSwordActionState::Acting;
        return;
    }

    ActionState = EMHGreatSwordActionState::Acting;
}

void UMHGreatSwordActionComponent::NotifyUtilityMoveEnded(const FGameplayTag& InMoveTag, const bool bInterrupted)
{
    if (ActiveUtilityMoveTag != InMoveTag)
    {
        return;
    }

    ActiveUtilityMoveTag = FGameplayTag();

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_Guard || InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawGuard)
    {
        if (!bGuardHeld)
        {
            ResetChargeFollowUpState();
            ActionState = EMHGreatSwordActionState::Neutral;
        }
        return;
    }

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawDefaultCharge
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlashCharging
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlashCharging
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlashCharging)
    {
        if (bInterrupted)
        {
            return;
        }

        ResetChargeContext();
        ActionState = EMHGreatSwordActionState::Neutral;
        return;
    }

    ResetChargeFollowUpState();
    ActionState = EMHGreatSwordActionState::Neutral;
}

void UMHGreatSwordActionComponent::NotifyActionFinished()
{
    if (IsCharging() || IsGuarding())
    {
        return;
    }

    ResetChargeFollowUpState();
    ActionState = EMHGreatSwordActionState::Neutral;
}

UAnimMontage* UMHGreatSwordActionComponent::ResolveMontageForMove(const FGameplayTag& InMoveTag) const
{
    const TSoftObjectPtr<UAnimMontage>* MontagePtr = MoveMontageMap.Find(InMoveTag);
    if (!MontagePtr || MontagePtr->IsNull())
    {
        return nullptr;
    }

    return MontagePtr->LoadSynchronous();
}

bool UMHGreatSwordActionComponent::FindAttackMetaRow(const FGameplayTag& InMoveTag, FMHAttackMetaRow& OutAttackMetaRow) const
{
    if (!IsValid(AttackMetaTable))
    {
        return false;
    }

    return UMHCombatDataLibrary::FindAttackMetaRowByTag(AttackMetaTable, InMoveTag, OutAttackMetaRow);
}

void UMHGreatSwordActionComponent::NotifyChargeLevelReached(const int32 InChargeLevel)
{
    if (!IsCharging())
    {
        return;
    }

    CurrentChargeLevel = FMath::Clamp(InChargeLevel, 0, 2);
    bChargeReleaseReady = true;

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("대검 차징 단계 갱신. Session=%d Family=%d Level=%d"),
        ChargeSessionId,
        static_cast<int32>(ChargeFamily),
        CurrentChargeLevel
    );
}

bool UMHGreatSwordActionComponent::NotifyChargeAutoReleaseRequested()
{
    if (!IsCharging())
    {
        return false;
    }

    bChargeAutoReleaseRequested = true;
    return FinalizeCurrentChargeRelease();
}

void UMHGreatSwordActionComponent::NotifyBeginAttackRollWindow()
{
    bAttackRollWindowOpen = true;
}

void UMHGreatSwordActionComponent::NotifyEndAttackRollWindow()
{
    bAttackRollWindowOpen = false;
}

void UMHGreatSwordActionComponent::OpenChargeFollowUpWindow(const FGameplayTag& InSourceMoveTag)
{
    bChargeFollowUpWindowOpen = InSourceMoveTag.IsValid();
    ChargeFollowUpSourceMoveTag = bChargeFollowUpWindowOpen ? InSourceMoveTag : FGameplayTag();
}

void UMHGreatSwordActionComponent::CloseChargeFollowUpWindow()
{
    ResetChargeFollowUpState();
}

FGameplayTag UMHGreatSwordActionComponent::ResolveDodgeMoveTag(const bool bInSheathed, const EMHDirectionalVariant InDirectionalVariant) const
{
    if (bInSheathed || !bAttackRollWindowOpen)
    {
        return MHGreatSwordGameplayTags::Move_GS_RollFront;
    }

    switch (InDirectionalVariant)
    {
    case EMHDirectionalVariant::Backward:
        return MHGreatSwordGameplayTags::Move_GS_RollBack;
    case EMHDirectionalVariant::Left:
        return MHGreatSwordGameplayTags::Move_GS_RollLeft;
    case EMHDirectionalVariant::Right:
        return MHGreatSwordGameplayTags::Move_GS_RollRight;
    case EMHDirectionalVariant::Forward:
    default:
        return MHGreatSwordGameplayTags::Move_GS_RollFront;
    }
}

bool UMHGreatSwordActionComponent::IsUtilityMoveTag(const FGameplayTag& InMoveTag) const
{
    return InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawOnly
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawDefaultCharge
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawGuard
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_Guard
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_Sheathe
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlashCharging
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlashCharging
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlashCharging
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollFront
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollBack
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollLeft
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_RollRight;
}

FGameplayTag UMHGreatSwordActionComponent::ResolveChargeChargingMoveTag(const EMHGreatSwordChargeFamily InFamily) const
{
    switch (InFamily)
    {
    case EMHGreatSwordChargeFamily::Charge:
        return MHGreatSwordGameplayTags::Move_GS_ChargeSlashCharging;
    case EMHGreatSwordChargeFamily::Strong:
        return MHGreatSwordGameplayTags::Move_GS_StrongChargeSlashCharging;
    case EMHGreatSwordChargeFamily::TrueCharge:
        return MHGreatSwordGameplayTags::Move_GS_TrueChargeSlashCharging;
    default:
        return FGameplayTag();
    }
}

FGameplayTag UMHGreatSwordActionComponent::ResolveChargeReleaseMoveTag(const EMHGreatSwordChargeFamily InFamily) const
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
        return FGameplayTag();
    }
}

EMHGreatSwordChargeFamily UMHGreatSwordActionComponent::ResolveNextChargeFamilyAfterTackleFromCharge(const EMHGreatSwordChargeFamily InCurrentFamily) const
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

void UMHGreatSwordActionComponent::ResetChargeContext()
{
    ChargeFamily = EMHGreatSwordChargeFamily::None;
    CurrentChargeLevel = 0;
    LastReleasedChargeLevel = 0;
    LastReleasedChargeFamily = EMHGreatSwordChargeFamily::None;
    bChargeReleaseReady = false;
    bChargeAutoReleaseRequested = false;
    bChargeStartedFromSheathedForwardInput = false;
    ResetChargeFollowUpState();
    ++ChargeSessionId;
}

void UMHGreatSwordActionComponent::ResetChargeFollowUpState()
{
    bChargeFollowUpWindowOpen = false;
    ChargeFollowUpSourceMoveTag = FGameplayTag();
}

void UMHGreatSwordActionComponent::QueuePendingMove(const FGameplayTag& InMoveTag, const EMHGreatSwordActionState InNextState)
{
    PendingMoveTag = InMoveTag;
    ActionState = InNextState;

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("대검 대기 기술 갱신. Move=%s State=%d"),
        *PendingMoveTag.ToString(),
        static_cast<int32>(ActionState)
    );
}

void UMHGreatSwordActionComponent::BeginCharging(const EMHGreatSwordChargeFamily InChargeFamily, const bool bInStartedFromSheathedForwardInput)
{
    ResetChargeContext();
    ChargeFamily = InChargeFamily;
    bChargeStartedFromSheathedForwardInput = bInStartedFromSheathedForwardInput;

    const FGameplayTag ChargingMoveTag = bInStartedFromSheathedForwardInput
        ? MHGreatSwordGameplayTags::Move_GS_DrawDefaultCharge
        : ResolveChargeChargingMoveTag(InChargeFamily);

    QueuePendingMove(ChargingMoveTag, EMHGreatSwordActionState::Charging);

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("대검 차징 시작. Session=%d Family=%d SheathedForward=%s"),
        ChargeSessionId,
        static_cast<int32>(ChargeFamily),
        bChargeStartedFromSheathedForwardInput ? TEXT("true") : TEXT("false")
    );
}

bool UMHGreatSwordActionComponent::FinalizeCurrentChargeRelease()
{
    if (!IsCharging())
    {
        return false;
    }

    const EMHGreatSwordChargeFamily ReleaseFamily = ChargeFamily;
    const int32 ReleaseLevel = FMath::Clamp(CurrentChargeLevel, 0, 2);
    const bool bUseDrawForwardSlash = bChargeStartedFromSheathedForwardInput && !bChargeReleaseReady;

    ChargeFamily = EMHGreatSwordChargeFamily::None;
    CurrentChargeLevel = 0;
    bChargeReleaseReady = false;
    bChargeAutoReleaseRequested = false;
    bChargeStartedFromSheathedForwardInput = false;
    ++ChargeSessionId;

    if (bUseDrawForwardSlash)
    {
        LastReleasedChargeFamily = EMHGreatSwordChargeFamily::None;
        LastReleasedChargeLevel = 0;
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    LastReleasedChargeFamily = ReleaseFamily;
    LastReleasedChargeLevel = ReleaseLevel;
    QueuePendingMove(ResolveChargeReleaseMoveTag(ReleaseFamily), EMHGreatSwordActionState::Acting);
    return PendingMoveTag.IsValid();
}
