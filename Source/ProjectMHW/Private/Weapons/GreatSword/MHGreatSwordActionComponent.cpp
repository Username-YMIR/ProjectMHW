#include "Weapons/GreatSword/MHGreatSwordActionComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/AnimMontage.h"
#include "Combat/Data/MHAttackMetaTypes.h"
#include "Combat/Data/MHCombatDataLibrary.h"
#include "GameplayTags/MHGreatSwordGameplayTags.h"
#include "Items/Instance/MHGreatSwordInstance.h"

DEFINE_LOG_CATEGORY(LogMHGreatSwordActionComponent);

namespace
{
    // 일반 모아베기 계열인지 확인한다.
    bool IsChargeSlashMove(const FGameplayTag& InMoveTag)
    {
        return InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv0
            || InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv1
            || InMoveTag == MHGreatSwordGameplayTags::Move_GS_ChargeSlash_Lv2;
    }

    // 강 모아베기 계열인지 확인한다.
    bool IsStrongChargeSlashMove(const FGameplayTag& InMoveTag)
    {
        return InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv0
            || InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv1
            || InMoveTag == MHGreatSwordGameplayTags::Move_GS_StrongChargeSlash_Lv2;
    }

    // 참 모아베기 계열인지 확인한다.
    bool IsTrueChargeSlashMove(const FGameplayTag& InMoveTag)
    {
        return InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv0
            || InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv1
            || InMoveTag == MHGreatSwordGameplayTags::Move_GS_TrueChargeSlash_Lv2;
    }
}

UMHGreatSwordActionComponent::UMHGreatSwordActionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMHGreatSwordActionComponent::TickComponent(
    const float DeltaTime,
    const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TryAutoReleaseCharge();
}

bool UMHGreatSwordActionComponent::HandlePrimaryPressed(const bool bInForwardInput, const bool bInSheathed)
{
    if (IsCharging())
    {
        return false;
    }

    if (!bInSheathed && ActionState == EMHGreatSwordActionState::Acting)
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

        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_DrawSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        BeginCharging(ResolveNextChargeFamilyAfterTackle());
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash)
    {
        if (bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::Strong);
            return true;
        }

        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsChargeSlashMove(LastCommittedMoveTag))
    {
        if (bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::Strong);
            return true;
        }

        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsStrongChargeSlashMove(LastCommittedMoveTag))
    {
        if (bInForwardInput)
        {
            BeginCharging(EMHGreatSwordChargeFamily::TrueCharge);
            return true;
        }

        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_WideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsTrueChargeSlashMove(LastCommittedMoveTag))
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_RisingSlash
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_WideSlash)
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

    FGameplayTag ReleaseMoveTag;
    if (bForwardDrawChargeEntry && ChargeLevel == 0)
    {
        ReleaseMoveTag = MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash;
    }
    else
    {
        ReleaseMoveTag = ResolveChargeReleaseMoveTag(ChargeFamily, ChargeLevel);
    }

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

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash, EMHGreatSwordActionState::Acting);
        return true;
    }

    if (IsTrueChargeSlashMove(LastCommittedMoveTag)
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash)
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_OverheadSlash)
    {
        QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_Tackle, EMHGreatSwordActionState::Acting);
        return true;
    }

    QueuePendingMove(MHGreatSwordGameplayTags::Move_GS_OverheadSlash, EMHGreatSwordActionState::Acting);
    return true;
}

bool UMHGreatSwordActionComponent::HandleWeaponSpecialPressed()
{
    if (IsCharging())
    {
        return false;
    }

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        return false;
    }

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
    if (IsCharging())
    {
        return false;
    }

    if (ActionState == EMHGreatSwordActionState::Acting)
    {
        return false;
    }

    if (LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_Tackle
        || LastCommittedMoveTag == MHGreatSwordGameplayTags::Move_GS_JumpingWideSlash
        || IsTrueChargeSlashMove(LastCommittedMoveTag))
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

    if (PendingMoveTag.IsValid())
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
    if (IsChargeSlashMove(LastCommittedMoveTag))
    {
        return EMHGreatSwordChargeFamily::Strong;
    }

    if (IsStrongChargeSlashMove(LastCommittedMoveTag) || IsTrueChargeSlashMove(LastCommittedMoveTag))
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

void UMHGreatSwordActionComponent::BeginCharging(const EMHGreatSwordChargeFamily InChargeFamily, const bool bInForwardDrawEntry)
{
    ChargeFamily = InChargeFamily;
    ChargeStartWorldSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    ActionState = EMHGreatSwordActionState::Charging;
    PendingMoveTag = FGameplayTag();
    bForwardDrawChargeEntry = bInForwardDrawEntry;
    SetComponentTickEnabled(true);

    UE_LOG(
        LogMHGreatSwordActionComponent,
        Verbose,
        TEXT("대검 차징 시작. Family=%d ForwardDraw=%d"),
        static_cast<int32>(ChargeFamily),
        bForwardDrawChargeEntry ? 1 : 0
    );
}

void UMHGreatSwordActionComponent::EndCharging()
{
    ChargeFamily = EMHGreatSwordChargeFamily::None;
    ChargeStartWorldSeconds = 0.0f;
    bForwardDrawChargeEntry = false;
    SetComponentTickEnabled(false);
}

bool UMHGreatSwordActionComponent::TryAutoReleaseCharge()
{
    if (!IsCharging())
    {
        return false;
    }

    const float CurrentWorldSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const float ElapsedSeconds = FMath::Max(0.0f, CurrentWorldSeconds - ChargeStartWorldSeconds);
    if (ElapsedSeconds < ChargeAutoReleaseSeconds)
    {
        return false;
    }

    const int32 ChargeLevel = ResolveChargeLevelFromElapsedSeconds(ElapsedSeconds);

    FGameplayTag ReleaseMoveTag;
    if (bForwardDrawChargeEntry && ChargeLevel == 0)
    {
        ReleaseMoveTag = MHGreatSwordGameplayTags::Move_GS_DrawForwardSlash;
    }
    else
    {
        ReleaseMoveTag = ResolveChargeReleaseMoveTag(ChargeFamily, ChargeLevel);
    }

    EndCharging();

    if (!ReleaseMoveTag.IsValid())
    {
        return false;
    }

    QueuePendingMove(ReleaseMoveTag, EMHGreatSwordActionState::Acting);
    return TryActivateQueuedMoveFromOwner();
}

bool UMHGreatSwordActionComponent::TryActivateQueuedMoveFromOwner() const
{
    if (!PendingMoveTag.IsValid())
    {
        return false;
    }

    const AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(GetOwner());
    if (!GreatSword)
    {
        return false;
    }

    const TSubclassOf<UGameplayAbility> AbilityClass = GreatSword->GetPrimaryAttackAbilityClass();
    if (!AbilityClass)
    {
        return false;
    }

    const AActor* OwnerActor = GreatSword->GetOwner();
    const IAbilitySystemInterface* AbilityOwner = Cast<IAbilitySystemInterface>(OwnerActor);
    if (!AbilityOwner)
    {
        return false;
    }

    UAbilitySystemComponent* ASC = AbilityOwner->GetAbilitySystemComponent();
    if (!ASC)
    {
        return false;
    }

    return ASC->TryActivateAbilityByClass(AbilityClass);
}
