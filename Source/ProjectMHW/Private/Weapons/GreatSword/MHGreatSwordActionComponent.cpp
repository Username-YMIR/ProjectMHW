#include "Weapons/GreatSword/MHGreatSwordActionComponent.h"

#include "Animation/AnimMontage.h"
#include "Combat/Data/MHAttackMetaTypes.h"
#include "Combat/Data/MHCombatDataLibrary.h"
#include "GameplayTags/MHGreatSwordGameplayTags.h"
#include "GameplayTags/MHInputPatternGameplayTags.h"
#include "Weapons/GreatSword/MHGreatSwordComboGraph.h"
#include "Weapons/GreatSword/MHGreatSwordComboTypes.h"
#include "Weapons/GreatSword/MHGreatSwordChargeStateComponent.h"

DEFINE_LOG_CATEGORY(LogMHGreatSwordActionComponent);

UMHGreatSwordActionComponent::UMHGreatSwordActionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMHGreatSwordActionComponent::SetComboGraph(UMHGreatSwordComboGraph* InComboGraph)
{
    ComboGraph = InComboGraph;
}

void UMHGreatSwordActionComponent::SetChargeStateComponent(UMHGreatSwordChargeStateComponent* InChargeStateComponent)
{
    ChargeStateComponent = InChargeStateComponent;
    SyncChargeStateComponentFromLegacyState();
}

bool UMHGreatSwordActionComponent::HandleResolvedInputPattern(const FGameplayTag& InPatternTag)
{
    using namespace MHInputPatternGameplayTags;

    if (!InPatternTag.IsValid())
    {
        return false;
    }

    if (InPatternTag == InputPattern_GS_PrimaryRelease || InPatternTag == InputPattern_GS_AutoRelease)
    {
        return FinalizeCurrentChargeRelease();
    }

    if (InPatternTag == InputPattern_GS_Secondary && IsCharging())
    {
        return HandleChargingSecondaryInput();
    }

    if (InPatternTag == InputPattern_GS_WeaponSpecialRelease)
    {
        return HandleWeaponSpecialReleased();
    }

    FMHGreatSwordActionDirective Directive;
    if (!TryResolveGraphDrivenDirective(InPatternTag, Directive))
    {
        if (ComboGraph)
        {
            UE_LOG(
                LogMHGreatSwordActionComponent,
                Verbose,
                TEXT("[Graph] Failed to resolve directive. Pattern=%s Move=%s Phase=%d"),
                *InPatternTag.ToString(),
                *LastCommittedMoveTag.ToString(),
                static_cast<int32>(ResolveTransitionPhase())
            );
        }
        else
        {
            UE_LOG(
                LogMHGreatSwordActionComponent,
                Verbose,
                TEXT("[Graph] Combo graph unavailable for pattern resolution. Pattern=%s Move=%s"),
                *InPatternTag.ToString(),
                *LastCommittedMoveTag.ToString()
            );
        }

        return false;
    }

    return ExecuteResolvedDirective(Directive);
}

bool UMHGreatSwordActionComponent::HandleInputPatternWithBuffering(const FGameplayTag& InPatternTag)
{
    using namespace MHInputPatternGameplayTags;

    if (!InPatternTag.IsValid())
    {
        return false;
    }

    if (ActionState == EMHGreatSwordActionState::Acting && !IsAnyTransitionWindowOpen() && !IsCharging() && !IsGuarding())
    {
        if (InPatternTag == InputPattern_GS_Primary || InPatternTag == InputPattern_GS_ForwardPrimary)
        {
            BufferInput(EMHGreatSwordBufferedInputType::Primary, InPatternTag == InputPattern_GS_ForwardPrimary);
            return true;
        }

        if (InPatternTag == InputPattern_GS_Secondary)
        {
            BufferInput(EMHGreatSwordBufferedInputType::Secondary, false);
            return true;
        }

        if (InPatternTag == InputPattern_GS_Simultaneous)
        {
            BufferInput(EMHGreatSwordBufferedInputType::Simultaneous, false);
            return true;
        }
    }

    return HandleResolvedInputPattern(InPatternTag);
}

bool UMHGreatSwordActionComponent::TryResolveGraphDrivenDirective(const FGameplayTag& InPatternTag, FMHGreatSwordActionDirective& OutDirective) const
{
    OutDirective.Reset();

    if (!ComboGraph)
    {
        return false;
    }

    const EMHGreatSwordTransitionPhase Phase = ResolveTransitionPhase();
    if (Phase == EMHGreatSwordTransitionPhase::None || Phase == EMHGreatSwordTransitionPhase::Charging)
    {
        return false;
    }

    FMHGreatSwordTransitionContext Context;
    Context.CurrentMoveTag = LastCommittedMoveTag;
    Context.ChargeFollowUpSourceMoveTag = ChargeFollowUpSourceMoveTag;
    Context.Phase = Phase;
    return ComboGraph->FindBestDirective(InPatternTag, Context, OutDirective);
}

bool UMHGreatSwordActionComponent::ExecuteResolvedDirective(const FMHGreatSwordActionDirective& InDirective)
{
    if (!InDirective.IsValid())
    {
        return false;
    }

    switch (InDirective.Type)
    {
    case EMHGreatSwordDirectiveType::PlayMove:
        if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle && InDirective.MoveTag != MHGreatSwordGameplayTags::Move_GS_Tackle)
        {
            ClearPostTackleChargeContext();
        }

        QueuePendingMove(InDirective.MoveTag, EMHGreatSwordActionState::Acting);
        return true;

    case EMHGreatSwordDirectiveType::BeginCharge:
    {
        EMHGreatSwordChargeFamily NextFamily = InDirective.ChargeFamilyHint;
        if (InDirective.bUsePostTackleChargeFamily)
        {
            if (ChargeStateComponent)
            {
                const EMHGreatSwordChargeFamily ConsumedFamily = ChargeStateComponent->ConsumePostTackleChargeFamily();
                if (ConsumedFamily != EMHGreatSwordChargeFamily::None)
                {
                    NextFamily = ConsumedFamily;
                }
            }

            if (NextFamily == EMHGreatSwordChargeFamily::None)
            {
                NextFamily = PendingPostTackleChargeFamily;
            }

            if (NextFamily == EMHGreatSwordChargeFamily::None)
            {
                NextFamily = ResolveNextChargeFamilyAfterTackleFromCharge(LastTackleSourceChargeFamily);
            }

            if (NextFamily == EMHGreatSwordChargeFamily::None)
            {
                NextFamily = EMHGreatSwordChargeFamily::Strong;
            }

            ClearPostTackleChargeContext();
        }

        if (NextFamily == EMHGreatSwordChargeFamily::None)
        {
            NextFamily = EMHGreatSwordChargeFamily::Charge;
        }

        BeginCharging(NextFamily, InDirective.bStartedFromSheathedForward);
        return true;
    }

    case EMHGreatSwordDirectiveType::EnterGuard:
        bGuardHeld = true;
        QueuePendingMove(InDirective.MoveTag, EMHGreatSwordActionState::Guarding);
        return true;

    case EMHGreatSwordDirectiveType::ExitGuard:
        return HandleWeaponSpecialReleased();

    default:
        return false;
    }
}

EMHGreatSwordTransitionPhase UMHGreatSwordActionComponent::ResolveTransitionPhase() const
{
    if (IsCharging())
    {
        return EMHGreatSwordTransitionPhase::Charging;
    }

    if (bChargeFollowUpWindowOpen)
    {
        return EMHGreatSwordTransitionPhase::ChargeFollowUp;
    }

    if (bEarlyTransitionWindowOpen)
    {
        return EMHGreatSwordTransitionPhase::EarlyTransition;
    }

    if (ActionState == EMHGreatSwordActionState::Neutral || !LastCommittedMoveTag.IsValid())
    {
        return EMHGreatSwordTransitionPhase::Entry;
    }

    return EMHGreatSwordTransitionPhase::None;
}


bool UMHGreatSwordActionComponent::IsAnyTransitionWindowOpen() const
{
    return bEarlyTransitionWindowOpen || bChargeFollowUpWindowOpen;
}

void UMHGreatSwordActionComponent::CaptureRuntimeSnapshot(FMHGreatSwordRuntimeSnapshot& OutSnapshot) const
{
    OutSnapshot.ActionState = ActionState;
    OutSnapshot.ChargeFamily = ChargeFamily;
    OutSnapshot.PendingMoveTag = PendingMoveTag;
    OutSnapshot.LastCommittedMoveTag = LastCommittedMoveTag;
    OutSnapshot.ActiveUtilityMoveTag = ActiveUtilityMoveTag;
    OutSnapshot.PendingPostTackleChargeFamily = PendingPostTackleChargeFamily;
    OutSnapshot.LastTackleSourceChargeFamily = LastTackleSourceChargeFamily;
    OutSnapshot.CurrentChargeLevel = CurrentChargeLevel;
    OutSnapshot.LastReleasedChargeLevel = LastReleasedChargeLevel;
    OutSnapshot.LastReleasedChargeFamily = LastReleasedChargeFamily;
    OutSnapshot.ChargeSessionId = ChargeSessionId;
    OutSnapshot.bChargeReleaseReady = bChargeReleaseReady;
    OutSnapshot.bChargeAutoReleaseRequested = bChargeAutoReleaseRequested;
    OutSnapshot.bChargeStartedFromSheathedForwardInput = bChargeStartedFromSheathedForwardInput;
    OutSnapshot.bGuardHeld = bGuardHeld;
    OutSnapshot.bAttackRollWindowOpen = bAttackRollWindowOpen;
    OutSnapshot.bEarlyTransitionWindowOpen = bEarlyTransitionWindowOpen;
    OutSnapshot.bChargeFollowUpWindowOpen = bChargeFollowUpWindowOpen;
    OutSnapshot.ChargeFollowUpSourceMoveTag = ChargeFollowUpSourceMoveTag;
    OutSnapshot.bHasBufferedInput = bHasBufferedInput;
    OutSnapshot.BufferedInputType = BufferedInputType;
    OutSnapshot.bBufferedForwardInput = bBufferedForwardInput;
}

void UMHGreatSwordActionComponent::RestoreRuntimeSnapshot(const FMHGreatSwordRuntimeSnapshot& InSnapshot)
{
    ActionState = InSnapshot.ActionState;
    ChargeFamily = InSnapshot.ChargeFamily;
    PendingMoveTag = InSnapshot.PendingMoveTag;
    LastCommittedMoveTag = InSnapshot.LastCommittedMoveTag;
    ActiveUtilityMoveTag = InSnapshot.ActiveUtilityMoveTag;
    PendingPostTackleChargeFamily = InSnapshot.PendingPostTackleChargeFamily;
    LastTackleSourceChargeFamily = InSnapshot.LastTackleSourceChargeFamily;
    CurrentChargeLevel = InSnapshot.CurrentChargeLevel;
    LastReleasedChargeLevel = InSnapshot.LastReleasedChargeLevel;
    LastReleasedChargeFamily = InSnapshot.LastReleasedChargeFamily;
    ChargeSessionId = InSnapshot.ChargeSessionId;
    bChargeReleaseReady = InSnapshot.bChargeReleaseReady;
    bChargeAutoReleaseRequested = InSnapshot.bChargeAutoReleaseRequested;
    bChargeStartedFromSheathedForwardInput = InSnapshot.bChargeStartedFromSheathedForwardInput;
    bGuardHeld = InSnapshot.bGuardHeld;
    bAttackRollWindowOpen = InSnapshot.bAttackRollWindowOpen;
    bEarlyTransitionWindowOpen = InSnapshot.bEarlyTransitionWindowOpen;
    bChargeFollowUpWindowOpen = InSnapshot.bChargeFollowUpWindowOpen;
    ChargeFollowUpSourceMoveTag = InSnapshot.ChargeFollowUpSourceMoveTag;
    bHasBufferedInput = InSnapshot.bHasBufferedInput;
    BufferedInputType = InSnapshot.BufferedInputType;
    bBufferedForwardInput = InSnapshot.bBufferedForwardInput;
    SyncChargeStateComponentFromLegacyState();
}

bool UMHGreatSwordActionComponent::HandleDodgePressed(const bool bInSheathed, const EMHDirectionalVariant InDirectionalVariant)
{
    if (bInSheathed)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_RollFront, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsCharging())
    {
        return false;
    }

    if (IsGuarding())
    {
        bGuardHeld = false;
        QueuePendingMove(ResolveDodgeMoveTag(false, InDirectionalVariant), EMHGreatSwordActionState::Acting);
        return true;
    }

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        if (!bAttackRollWindowOpen)
        {
            return false;
        }

        QueuePendingMove(ResolveDodgeMoveTag(false, InDirectionalVariant), EMHGreatSwordActionState::Acting);
        return true;
    }

    if (ActionState == EMHGreatSwordActionState::Neutral)
    {
        QueuePendingMove(ResolveDodgeMoveTag(false, InDirectionalVariant), EMHGreatSwordActionState::Acting);
        return true;
    }

    return false;
}

bool UMHGreatSwordActionComponent::HandlePrimaryPressed(const bool bInForwardInput, const bool bInSheathed)
{
    if (IsCharging() || IsGuarding())
    {
        return false;
    }

    const bool bUseForwardPrimary = !bInSheathed
        && bInForwardInput
        && ActionState == EMHGreatSwordActionState::Acting
        && IsAnyTransitionWindowOpen();

    const FGameplayTag PatternTag = bInSheathed
        ? (bInForwardInput ? MHInputPatternGameplayTags::InputPattern_GS_DrawCharge : MHInputPatternGameplayTags::InputPattern_GS_DrawOnly)
        : (bUseForwardPrimary ? MHInputPatternGameplayTags::InputPattern_GS_ForwardPrimary : MHInputPatternGameplayTags::InputPattern_GS_Primary);

    return HandleInputPatternWithBuffering(PatternTag);
}

bool UMHGreatSwordActionComponent::HandlePrimaryReleased()
{
    return HandleInputPatternWithBuffering(MHInputPatternGameplayTags::InputPattern_GS_PrimaryRelease);
}

bool UMHGreatSwordActionComponent::HandleSecondaryPressed()
{
    if (IsGuarding())
    {
        return false;
    }

    if (IsCharging())
    {
        return HandleChargingSecondaryInput();
    }

    return HandleInputPatternWithBuffering(MHInputPatternGameplayTags::InputPattern_GS_Secondary);
}

bool UMHGreatSwordActionComponent::HandleWeaponSpecialPressed(const bool bInSheathed)
{
    if (IsCharging() || IsGuarding())
    {
        return false;
    }

    const FGameplayTag PatternTag = bInSheathed
        ? MHInputPatternGameplayTags::InputPattern_GS_DrawGuard
        : MHInputPatternGameplayTags::InputPattern_GS_WeaponSpecial;

    return HandleInputPatternWithBuffering(PatternTag);
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

    return HandleInputPatternWithBuffering(MHInputPatternGameplayTags::InputPattern_GS_Simultaneous);
}

FGameplayTag UMHGreatSwordActionComponent::ConsumePendingMoveTag()
{
    const FGameplayTag ResultTag = PendingMoveTag;
    PendingMoveTag = FGameplayTag();
    return ResultTag;
}

float UMHGreatSwordActionComponent::ResolveChargeDamageScaleForMove(const FGameplayTag& InMoveTag) const
{
    const EMHGreatSwordChargeFamily DamageFamily = ChargeStateComponent ? ChargeStateComponent->GetLastReleasedChargeFamily() : LastReleasedChargeFamily;
    const int32 DamageLevel = ChargeStateComponent ? ChargeStateComponent->GetLastReleasedChargeLevel() : LastReleasedChargeLevel;

    const auto ResolveScale = [DamageLevel](const FVector& InScaleVector) -> float
    {
        switch (FMath::Clamp(DamageLevel, 0, 2))
        {
        case 0:
            return InScaleVector.X;
        case 1:
            return InScaleVector.Y;
        default:
            return InScaleVector.Z;
        }
    };

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash && DamageFamily == EMHGreatSwordChargeFamily::Charge)
    {
        return ResolveScale(ChargeSlashLevelDamageScale);
    }

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash && DamageFamily == EMHGreatSwordChargeFamily::Strong)
    {
        return ResolveScale(StrongChargeSlashLevelDamageScale);
    }

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash && DamageFamily == EMHGreatSwordChargeFamily::TrueCharge)
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
        ClearPostTackleChargeContext();
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

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_GuardImpact)
    {
        LastCommittedMoveTag = FGameplayTag();
        bGuardHeld = false;
        ClearPostTackleChargeContext();
        ActionState = EMHGreatSwordActionState::Acting;
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
        ClearPostTackleChargeContext();
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
    ClearPostTackleChargeContext();
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

    if (InMoveTag == MHGreatSwordGameplayTags::Move_GS_GuardImpact)
    {
        ActionState = EMHGreatSwordActionState::Neutral;
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
    ClearPostTackleChargeContext();
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

    if (ChargeStateComponent)
    {
        ChargeStateComponent->NotifyChargeLevelReached(InChargeLevel);
    }

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("[Charge] Level reached. Session=%d Family=%d Level=%d"),
        ChargeSessionId,
        static_cast<int32>(ChargeFamily),
        CurrentChargeLevel
    );
}

bool UMHGreatSwordActionComponent::NotifyChargeAutoReleaseRequested()
{
    using namespace MHInputPatternGameplayTags;

    if (!IsCharging())
    {
        return false;
    }

    bChargeAutoReleaseRequested = true;
    return HandleResolvedInputPattern(InputPattern_GS_AutoRelease);
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
        || InMoveTag == MHGreatSwordGameplayTags::Move_GS_GuardImpact
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
    if (ChargeStateComponent)
    {
        ChargeStateComponent->ResetChargeState();
    }
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

void UMHGreatSwordActionComponent::ClearPostTackleChargeContext()
{
    PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
    LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;

    if (ChargeStateComponent)
    {
        ChargeStateComponent->ClearPostTackleChargeState();
    }
}

void UMHGreatSwordActionComponent::SyncChargeStateComponentFromLegacyState()
{
    if (!ChargeStateComponent)
    {
        return;
    }

    ChargeStateComponent->SyncFromMirroredState(
        ChargeFamily,
        CurrentChargeLevel,
        PendingPostTackleChargeFamily,
        LastTackleSourceChargeFamily,
        LastReleasedChargeFamily,
        LastReleasedChargeLevel,
        bChargeReleaseReady,
        bChargeStartedFromSheathedForwardInput
    );
}

void UMHGreatSwordActionComponent::ClearBufferedInput()
{
    bHasBufferedInput = false;
    BufferedInputType = EMHGreatSwordBufferedInputType::None;
    bBufferedForwardInput = false;
}

void UMHGreatSwordActionComponent::BufferInput(const EMHGreatSwordBufferedInputType InInputType, const bool bInForwardInput)
{
    bHasBufferedInput = true;
    BufferedInputType = InInputType;
    bBufferedForwardInput = bInForwardInput;

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("[Buffer] Stored transition input. Type=%d Forward=%d Move=%s"),
        static_cast<int32>(BufferedInputType),
        bBufferedForwardInput ? 1 : 0,
        *LastCommittedMoveTag.ToString()
    );
}

bool UMHGreatSwordActionComponent::TryConsumeBufferedTransitionInput()
{
    using namespace MHInputPatternGameplayTags;

    if (!bHasBufferedInput || !IsAnyTransitionWindowOpen() || ActionState != EMHGreatSwordActionState::Acting)
    {
        return false;
    }

    const EMHGreatSwordBufferedInputType BufferedType = BufferedInputType;
    const bool bForwardInput = bBufferedForwardInput;
    ClearBufferedInput();

    FGameplayTag PatternTag;
    switch (BufferedType)
    {
    case EMHGreatSwordBufferedInputType::Primary:
        PatternTag = bForwardInput ? InputPattern_GS_ForwardPrimary : InputPattern_GS_Primary;
        break;
    case EMHGreatSwordBufferedInputType::Secondary:
        PatternTag = InputPattern_GS_Secondary;
        break;
    case EMHGreatSwordBufferedInputType::Simultaneous:
        PatternTag = InputPattern_GS_Simultaneous;
        break;
    default:
        break;
    }

    if (!PatternTag.IsValid())
    {
        return false;
    }

    if (HandleResolvedInputPattern(PatternTag))
    {
        return true;
    }

    if (ComboGraph)
    {
        return false;
    }

    return TryConsumeLegacyBufferedTransitionInput(BufferedType, bForwardInput);
}

bool UMHGreatSwordActionComponent::TryConsumeLegacyBufferedTransitionInput(
    const EMHGreatSwordBufferedInputType InBufferedType,
    const bool bInForwardInput)
{
    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("[LegacyFallback] Resolve buffered transition without combo graph. Type=%d Move=%s"),
        static_cast<int32>(InBufferedType),
        *LastCommittedMoveTag.ToString()
    );

    switch (InBufferedType)
    {
    case EMHGreatSwordBufferedInputType::Primary:
        return ResolvePrimaryDuringEarlyTransition(bInForwardInput);
    case EMHGreatSwordBufferedInputType::Secondary:
        return ResolveSecondaryDuringEarlyTransition();
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
        // 콤보 그래프가 없을 때만 태클 후속 분기를 레거시 호환 경로로 유지한다.
        if (!bInForwardInput)
        {
            ClearPostTackleChargeContext();

            UE_LOG(
                LogMHGreatSwordActionComponent,
                Verbose,
                TEXT("[LegacyFallback] Tackle follow-up resolved to WideSlash.")
            );

            QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
            return true;
        }

        EMHGreatSwordChargeFamily NextFamily = PendingPostTackleChargeFamily;

        if (NextFamily == EMHGreatSwordChargeFamily::None)
        {
            NextFamily = ResolveNextChargeFamilyAfterTackleFromCharge(LastTackleSourceChargeFamily);
        }

        if (NextFamily == EMHGreatSwordChargeFamily::None)
        {
            NextFamily = EMHGreatSwordChargeFamily::Strong;
        }

        ClearPostTackleChargeContext();

        UE_LOG(
            LogMHGreatSwordActionComponent,
            Verbose,
            TEXT("[LegacyFallback] Tackle follow-up resolved to charge. Family=%d"),
            static_cast<int32>(NextFamily)
        );

        BeginCharging(NextFamily, false);
        return true;
    }

    if (bChargeFollowUpWindowOpen)
    {
        if (ChargeFollowUpSourceMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash)
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

        if (ChargeFollowUpSourceMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash)
        {
            if (bInForwardInput)
            {
                BeginCharging(EMHGreatSwordChargeFamily::TrueCharge, false);
            }
            else
            {
                QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
            }
            return true;
        }

        if (ChargeFollowUpSourceMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash)
        {
            return false;
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

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash)
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

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash)
    {
        if (bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::TrueCharge, false);
        }
        else
        {
            QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        }
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash)
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash
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
        ClearPostTackleChargeContext();
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash, EMHGreatSwordActionState::Acting);
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

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
        return true;
    }

    return false;
}

bool UMHGreatSwordActionComponent::ResolveSimultaneousDuringEarlyTransition()
{
    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_WideSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_RisingSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    return false;
}

void UMHGreatSwordActionComponent::QueuePendingMove(const FGameplayTag& InMoveTag, const EMHGreatSwordActionState InNextState)
{
    PendingMoveTag = InMoveTag;
    ActionState = InNextState;
    ClearBufferedInput();

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("[Queue] Pending move queued. Move=%s State=%d"),
        *PendingMoveTag.ToString(),
        static_cast<int32>(ActionState)
    );
}

void UMHGreatSwordActionComponent::BeginCharging(const EMHGreatSwordChargeFamily InChargeFamily, const bool bInStartedFromSheathedForwardInput)
{
    ResetChargeContext();
    ChargeFamily = InChargeFamily;

    if (ChargeStateComponent)
    {
        ChargeStateComponent->BeginCharge(InChargeFamily, bInStartedFromSheathedForwardInput);
    }
    bChargeStartedFromSheathedForwardInput = bInStartedFromSheathedForwardInput;

    const FGameplayTag ChargingMoveTag = bInStartedFromSheathedForwardInput
        ? MHGreatSwordGameplayTags::Move_GS_DrawDefaultCharge
        : ResolveChargeChargingMoveTag(InChargeFamily);

    QueuePendingMove(ChargingMoveTag, EMHGreatSwordActionState::Charging);

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("[Charge] Begin charging. Session=%d Family=%d SheathedForward=%s"),
        ChargeSessionId,
        static_cast<int32>(ChargeFamily),
        bChargeStartedFromSheathedForwardInput ? TEXT("true") : TEXT("false")
    );
}

bool UMHGreatSwordActionComponent::HandleChargingSecondaryInput()
{
    if (!IsCharging())
    {
        return false;
    }

    if (ChargeStateComponent)
    {
        ChargeStateComponent->HandleTackleFromCurrentCharge();
    }

    LastTackleSourceChargeFamily = ChargeFamily;
    PendingPostTackleChargeFamily = ResolveNextChargeFamilyAfterTackleFromCharge(ChargeFamily);
    ResetChargeContext();
    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
    return true;
}

bool UMHGreatSwordActionComponent::FinalizeCurrentChargeRelease()
{
    if (!IsCharging())
    {
        return false;
    }

    FMHGreatSwordActionDirective ReleaseDirective;
    const bool bHasReleaseDirective = ChargeStateComponent && ChargeStateComponent->BuildReleaseDirective(ReleaseDirective);

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

    if (bHasReleaseDirective && ReleaseDirective.Type == EMHGreatSwordDirectiveType::PlayMove && ReleaseDirective.MoveTag.IsValid())
    {
        if (ReleaseDirective.MoveTag == MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash)
        {
            LastReleasedChargeFamily = EMHGreatSwordChargeFamily::None;
            LastReleasedChargeLevel = 0;
        }
        else
        {
            LastReleasedChargeFamily = ChargeStateComponent->GetLastReleasedChargeFamily();
            LastReleasedChargeLevel = ChargeStateComponent->GetLastReleasedChargeLevel();
        }

        QueuePendingMove(ReleaseDirective.MoveTag, EMHGreatSwordActionState::Acting);
        return true;
    }

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















