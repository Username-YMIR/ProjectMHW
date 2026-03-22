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

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        if (!bEarlyTransitionWindowOpen)
        {
            BufferInput(EMHGreatSwordBufferedInputType::Primary, bInForwardInput, bInSheathed);
            return true;
        }

        return ResolvePrimaryDuringEarlyTransition(bInForwardInput);
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
        LastTackleSourceChargeFamily = ChargeFamily;
        PendingPostTackleChargeFamily = ResolveNextChargeFamilyAfterTackleFromCharge(ChargeFamily);
        ResetChargeContext();
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsGuarding())
    {
        return false;
    }

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        if (!bEarlyTransitionWindowOpen)
        {
            BufferInput(EMHGreatSwordBufferedInputType::Secondary, false, false);
            return true;
        }

        return ResolveSecondaryDuringEarlyTransition();
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

    if (IsGuarding())
    {
        return false;
    }

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        if (!bEarlyTransitionWindowOpen)
        {
            BufferInput(EMHGreatSwordBufferedInputType::WeaponSpecial, false, bInSheathed);
            return true;
        }

        return ResolveWeaponSpecialDuringEarlyTransition();
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

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        if (!bEarlyTransitionWindowOpen)
        {
            BufferInput(EMHGreatSwordBufferedInputType::Simultaneous, false, false);
            return true;
        }

        return ResolveSimultaneousDuringEarlyTransition();
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
    if (InMoveTag != MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
        LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
    }

    bEarlyTransitionWindowOpen = false;
    bAttackRollWindowOpen = false;
    ClearBufferedInput();
    ActionState = EMHGreatSwordActionState::Acting;
}

void UMHGreatSwordActionComponent::NotifyUtilityMoveStarted(const FGameplayTag& InMoveTag)
{
    ActiveUtilityMoveTag = InMoveTag;
    PendingMoveTag = FGameplayTag();
    bEarlyTransitionWindowOpen = false;
    bAttackRollWindowOpen = false;
    ClearBufferedInput();
    NotifyEndChargeFollowUpWindow();

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_Guard || InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawGuard)
    {
        LastCommittedMoveTag = FGameplayTag();
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
        PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
        LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
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
    PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
    LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
    bEarlyTransitionWindowOpen = false;
    bAttackRollWindowOpen = false;
    ClearBufferedInput();
    NotifyEndChargeFollowUpWindow();

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_Guard || InMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawGuard)
    {
        if (!bGuardHeld)
        {
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

    ActionState = EMHGreatSwordActionState::Neutral;
}

void UMHGreatSwordActionComponent::NotifyActionFinished()
{
    if (IsCharging() || IsGuarding())
    {
        return;
    }

    bEarlyTransitionWindowOpen = false;
    bAttackRollWindowOpen = false;
    PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
    LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
    ClearBufferedInput();
    NotifyEndChargeFollowUpWindow();
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

void UMHGreatSwordActionComponent::NotifyBeginEarlyTransitionWindow()
{
    bEarlyTransitionWindowOpen = true;
    TryConsumeBufferedTransitionInput();
}

void UMHGreatSwordActionComponent::NotifyEndEarlyTransitionWindow()
{
    bEarlyTransitionWindowOpen = false;
}

void UMHGreatSwordActionComponent::NotifyBeginChargeFollowUpWindow(const FGameplayTag& InSourceMoveTag)
{
    bChargeFollowUpWindowOpen = true;
    ChargeFollowUpSourceMoveTag = InSourceMoveTag;
    TryConsumeBufferedTransitionInput();
}

void UMHGreatSwordActionComponent::NotifyEndChargeFollowUpWindow()
{
    bChargeFollowUpWindowOpen = false;
    ChargeFollowUpSourceMoveTag = FGameplayTag();
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
    NotifyEndChargeFollowUpWindow();
    ++ChargeSessionId;
}

void UMHGreatSwordActionComponent::ClearBufferedInput()
{
    bHasBufferedInput = false;
    BufferedInputType = EMHGreatSwordBufferedInputType::None;
    bBufferedForwardInput = false;
    bBufferedSheathed = false;
}

void UMHGreatSwordActionComponent::BufferInput(const EMHGreatSwordBufferedInputType InInputType, const bool bInForwardInput, const bool bInSheathed)
{
    bHasBufferedInput = true;
    BufferedInputType = InInputType;
    bBufferedForwardInput = bInForwardInput;
    bBufferedSheathed = bInSheathed;

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("대검 입력 버퍼 저장. Input=%d Forward=%d Sheathed=%d Move=%s"),
        static_cast<int32>(BufferedInputType),
        bBufferedForwardInput ? 1 : 0,
        bBufferedSheathed ? 1 : 0,
        *LastCommittedMoveTag.ToString()
    );
}

bool UMHGreatSwordActionComponent::TryConsumeBufferedTransitionInput()
{
    if (!bHasBufferedInput || !bEarlyTransitionWindowOpen || ActionState != EMHGreatSwordActionState::Acting)
    {
        return false;
    }

    const EMHGreatSwordBufferedInputType BufferedType = BufferedInputType;
    const bool bForwardInput = bBufferedForwardInput;
    ClearBufferedInput();

    switch (BufferedType)
    {
    case EMHGreatSwordBufferedInputType::Primary:
        return ResolvePrimaryDuringEarlyTransition(bForwardInput);
    case EMHGreatSwordBufferedInputType::Secondary:
        return ResolveSecondaryDuringEarlyTransition();
    case EMHGreatSwordBufferedInputType::WeaponSpecial:
        return ResolveWeaponSpecialDuringEarlyTransition();
    case EMHGreatSwordBufferedInputType::Simultaneous:
        return ResolveSimultaneousDuringEarlyTransition();
    default:
        return false;
    }
}

bool UMHGreatSwordActionComponent::ResolvePrimaryDuringEarlyTransition(const bool bInForwardInput)
{
    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        EMHGreatSwordChargeFamily NextFamily = PendingPostTackleChargeFamily;

        if (NextFamily == EMHGreatSwordChargeFamily::None)
        {
            NextFamily = ResolveNextChargeFamilyAfterTackleFromCharge(LastTackleSourceChargeFamily);
        }

        if (NextFamily == EMHGreatSwordChargeFamily::None)
        {
            NextFamily = EMHGreatSwordChargeFamily::Strong;
        }

        PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
        LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
        BeginCharging(NextFamily, false);
        return true;
    }

    if (bChargeFollowUpWindowOpen)
    {
        if (ChargeFollowUpSourceMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash)
        {
            BeginCharging(EMHGreatSwordChargeFamily::Strong, false);
            return true;
        }

        if (ChargeFollowUpSourceMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash)
        {
            BeginCharging(EMHGreatSwordChargeFamily::TrueCharge, false);
            return true;
        }
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

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_RisingSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_WideSlash)
    {
        BeginCharging(EMHGreatSwordChargeFamily::Charge, false);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    return false;
}

bool UMHGreatSwordActionComponent::ResolveSecondaryDuringEarlyTransition()
{
    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
        LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_WideSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_RisingSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_OverheadSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
        return true;
    }

    return false;
}

bool UMHGreatSwordActionComponent::ResolveWeaponSpecialDuringEarlyTransition()
{
    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
        LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    bGuardHeld = true;
    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Guard, EMHGreatSwordActionState::Guarding);
    return true;
}

bool UMHGreatSwordActionComponent::ResolveSimultaneousDuringEarlyTransition()
{
    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_RisingSlash, EMHGreatSwordActionState::Acting);
    return true;
}

void UMHGreatSwordActionComponent::QueuePendingMove(const FGameplayTag& InMoveTag, const EMHGreatSwordActionState InNextState)
{
    PendingMoveTag = InMoveTag;
    ActionState = InNextState;
    ClearBufferedInput();

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
    NotifyEndChargeFollowUpWindow();
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
