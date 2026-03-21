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
    if (IsCharging())
    {
        return false;
    }

    if (bInSheathed)
    {
        if (bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::Charge);
            return true;
        }

        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_DrawSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        BeginCharging(ResolveNextChargeFamilyAfterTackle());
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash && bInForwardInput)
    {
        BeginCharging(EMHGreatSwordChargeFamily::Strong);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_WideSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_RisingSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash)
    {
        BeginCharging(EMHGreatSwordChargeFamily::Charge);
        return true;
    }

    BeginCharging(EMHGreatSwordChargeFamily::Charge);
    return true;
}

bool UMHGreatSwordActionComponent::HandlePrimaryReleased()
{
    if (!IsCharging())
    {
        return false;
    }

    const float CurrentWorldSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const float ElapsedSeconds = FMath::Max(0.0f, CurrentWorldSeconds - ChargeStartWorldSeconds);
    const int32 ChargeLevel = ResolveChargeLevelFromElapsedSeconds(ElapsedSeconds);
    const FGameplayTag ReleaseMoveTag = ResolveChargeReleaseMoveTag(ChargeFamily, ChargeLevel);
    EndCharging();

    if (!ReleaseMoveTag.IsValid())
    {
        return false;
    }

    QueuePendingMove(ReleaseMoveTag, EMHGreatSwordActionState::Acting);
    return true;
}

bool UMHGreatSwordActionComponent::HandleSecondaryPressed()
{
    if (IsCharging())
    {
        EndCharging();
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        return false;
    }

    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
    return true;
}

bool UMHGreatSwordActionComponent::HandleWeaponSpecialPressed()
{
    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Guard, EMHGreatSwordActionState::Acting);
    return true;
}

bool UMHGreatSwordActionComponent::HandleSimultaneousPressed()
{
    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_RisingSlash, EMHGreatSwordActionState::Acting);
    return true;
}

FGameplayTag UMHGreatSwordActionComponent::ConsumePendingMoveTag()
{
    const FGameplayTag ResultTag = PendingMoveTag;
    PendingMoveTag = FGameplayTag();
    return ResultTag;
}

int32 UMHGreatSwordActionComponent::GetChargeLevel() const
{
    if (!IsCharging())
    {
        return 0;
    }

    const float CurrentWorldSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const float ElapsedSeconds = FMath::Max(0.0f, CurrentWorldSeconds - ChargeStartWorldSeconds);
    return ResolveChargeLevelFromElapsedSeconds(ElapsedSeconds);
}

void UMHGreatSwordActionComponent::CommitExecutedMove(const FGameplayTag& InMoveTag)
{
    LastCommittedMoveTag = InMoveTag;
    PendingMoveTag = FGameplayTag();

    if (!IsCharging())
    {
        ActionState = EMHGreatSwordActionState::Acting;
    }
}

void UMHGreatSwordActionComponent::NotifyActionFinished()
{
    if (IsCharging())
    {
        return;
    }

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

FGameplayTag UMHGreatSwordActionComponent::ResolveChargeReleaseMoveTag(const EMHGreatSwordChargeFamily InFamily, const int32 InChargeLevel) const
{
    const int32 ClampedChargeLevel = FMath::Clamp(InChargeLevel, 0, 2);

    switch (InFamily)
    {
    case EMHGreatSwordChargeFamily::Charge:
        if (ClampedChargeLevel == 0)
        {
            return MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv0;
        }
        if (ClampedChargeLevel == 1)
        {
            return MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv1;
        }
        return MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv2;

    case EMHGreatSwordChargeFamily::Strong:
        if (ClampedChargeLevel == 0)
        {
            return MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv0;
        }
        if (ClampedChargeLevel == 1)
        {
            return MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv1;
        }
        return MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv2;

    case EMHGreatSwordChargeFamily::TrueCharge:
        if (ClampedChargeLevel == 0)
        {
            return MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv0;
        }
        if (ClampedChargeLevel == 1)
        {
            return MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv1;
        }
        return MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv2;

    default:
        return FGameplayTag();
    }
}

EMHGreatSwordChargeFamily UMHGreatSwordActionComponent::ResolveNextChargeFamilyAfterTackle() const
{
    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv0
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv1
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv2)
    {
        return EMHGreatSwordChargeFamily::Strong;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv0
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv1
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv2
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv0
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv1
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv2)
    {
        return EMHGreatSwordChargeFamily::TrueCharge;
    }

    return EMHGreatSwordChargeFamily::Strong;
}

int32 UMHGreatSwordActionComponent::ResolveChargeLevelFromElapsedSeconds(const float InElapsedSeconds) const
{
    if (InElapsedSeconds >= ChargeLevel2Seconds)
    {
        return 2;
    }

    if (InElapsedSeconds >= ChargeLevel1Seconds)
    {
        return 1;
    }

    return 0;
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

void UMHGreatSwordActionComponent::BeginCharging(const EMHGreatSwordChargeFamily InChargeFamily)
{
    ChargeFamily = InChargeFamily;
    ChargeStartWorldSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    ActionState = EMHGreatSwordActionState::Charging;
    PendingMoveTag = FGameplayTag();

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("대검 차징 시작. Family=%d"),
        static_cast<int32>(ChargeFamily)
    );
}

void UMHGreatSwordActionComponent::EndCharging()
{
    ChargeFamily = EMHGreatSwordChargeFamily::None;
    ChargeStartWorldSeconds = 0.0f;
}
