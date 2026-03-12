#include "AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Engine/World.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"
#include "Items/Instance/MHLongSwordInstance.h"
#include "TimerManager.h"
#include "Weapons/LongSword/MHLongSwordComboComponent.h"
#include "Weapons/LongSword/MHLongSwordComboGraph.h"

DEFINE_LOG_CATEGORY(LogMHGALSCombo);

UMHGA_LongSwordCombo::UMHGA_LongSwordCombo()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UMHGA_LongSwordCombo::TryEvaluateEarlyTransitionNow()
{
    if (!CanEvaluateEarlyTransition())
    {
        return false;
    }

    return TryCommitQueuedComboTransition();
}

void UMHGA_LongSwordCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!Player)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AMHLongSwordInstance* Weapon = Cast<AMHLongSwordInstance>(Player->GetEquippedWeapon());
    if (!Weapon)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UMHLongSwordComboComponent* ComboComp = Weapon->GetComboComponent();
    if (!ComboComp || !ComboComp->GetComboGraph())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CachedPlayer = Player;
    CachedWeapon = Weapon;
    CachedComboComponent = ComboComp;
    bIgnoreMontageCallbacks = false;

    const FGameplayTag PatternTag = ComboComp->ConsumeBufferedInputPattern();
    if (!PatternTag.IsValid() || !PlayNextMove(Player, Weapon, ComboComp, PatternTag))
    {
        Player->HandleComboMontageStateTransition(true);
        Player->ClearLongSwordForesightCounterSuccess();
        ComboComp->ResetCombo();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

void UMHGA_LongSwordCombo::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ClearTransitionPollingTimer();
    ClearTask();

    ActiveMontage = nullptr;
    bHasActiveNode = false;
    bIgnoreMontageCallbacks = false;

    CachedComboComponent = nullptr;
    CachedPlayer = nullptr;
    CachedWeapon = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UMHGA_LongSwordCombo::PlayNextMove(
    AMHPlayerCharacter* Player,
    AMHLongSwordInstance* Weapon,
    UMHLongSwordComboComponent* ComboComp,
    const FGameplayTag& InPatternTag)
{
    if (!Player || !Weapon || !ComboComp || !InPatternTag.IsValid())
    {
        return false;
    }

    const FGameplayTag PreviousMoveTag = ComboComp->GetCurrentMoveTag();
    const bool bCounterSuccess = Player->HasLongSwordForesightCounterSuccess();

    const FMHLongSwordComboNode* Node = ComboComp->SelectNextNode(InPatternTag, bCounterSuccess);
    if (!Node)
    {
        UE_LOG(LogMHGALSCombo, Verbose, TEXT("다음 콤보 노드를 찾지 못했습니다. Current=%s Pattern=%s"), *PreviousMoveTag.ToString(), *InPatternTag.ToString());
        return false;
    }

    ComboComp->CommitMove(*Node);
    return PlayResolvedNode(*Node, PreviousMoveTag);
}

bool UMHGA_LongSwordCombo::PlayResolvedNode(const FMHLongSwordComboNode& InNode, const FGameplayTag& InPreviousMoveTag)
{
    if (!CachedPlayer || !CachedWeapon || !CachedComboComponent)
    {
        return false;
    }

    UAnimMontage* Montage = InNode.Montage.IsNull() ? nullptr : InNode.Montage.LoadSynchronous();

    // Fade / LateralFade처럼 방향 Variant가 필요한 기술은
    // 플레이어 상태를 기준으로 최종 재생 몽타주를 한 번 더 치환한다.
    Montage = CachedPlayer->ResolveLongSwordMoveMontageOverride(InNode.MoveTag, Montage);
    if (!Montage)
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("Montage is null. MoveTag=%s"), *InNode.MoveTag.ToString());
        return false;
    }

    if (InPreviousMoveTag == MHLongSwordGameplayTags::Move_LS_ForesightSlash)
    {
        CachedPlayer->ClearLongSwordForesightCounterSuccess();
    }

    // 이전 드로우 엔트리 기술에서 조기 전환이 일어나면, 발도 상태 전환을 여기서 확정한다.
    if (IsDrawEntryMoveTag(InPreviousMoveTag))
    {
        CachedPlayer->HandleComboMontageStateTransition(false);
    }

    ClearTask();
    ActiveMontage = Montage;
    ActiveNode = InNode;
    bHasActiveNode = true;
    bIgnoreMontageCallbacks = false;

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.0f, InNode.SectionName, true);
    if (!MontageTask)
    {
        return false;
    }

    MontageTask->OnCompleted.AddDynamic(this, &UMHGA_LongSwordCombo::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UMHGA_LongSwordCombo::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UMHGA_LongSwordCombo::OnMontageInterrupted);
    MontageTask->ReadyForActivation();

    ClearTransitionPollingTimer();
    if (UWorld* World = GetWorld())
    {
        // 조기 전환은 몽타주 재생 중 주기적으로 검사한다.
        World->GetTimerManager().SetTimer(TransitionPollingTimerHandle, this, &UMHGA_LongSwordCombo::PollQueuedComboTransition, 0.01f, true);
    }

    UE_LOG(LogMHGALSCombo, Verbose, TEXT("콤보 몽타주 재생 시작. Move=%s"), *InNode.MoveTag.ToString());
    return true;
}

bool UMHGA_LongSwordCombo::CanEvaluateEarlyTransition() const
{
    if (!CachedPlayer || !CachedComboComponent || !ActiveMontage || !bHasActiveNode)
    {
        return false;
    }

    if (!ActiveNode.bAllowEarlyTransition)
    {
        return false;
    }

    if (!CachedComboComponent->HasAcceptedBufferedInputPattern())
    {
        return false;
    }

    if (ActiveNode.bUseNotifyDrivenEarlyTransition)
    {
        return CachedComboComponent->IsEarlyTransitionWindowOpen();
    }

    return true;
}

void UMHGA_LongSwordCombo::PollQueuedComboTransition()
{
    if (!CanEvaluateEarlyTransition())
    {
        return;
    }

    if (ActiveNode.bUseNotifyDrivenEarlyTransition)
    {
        TryCommitQueuedComboTransition();
        return;
    }

    UAnimInstance* AnimInstance = CachedPlayer->GetMesh() ? CachedPlayer->GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        return;
    }

    const float MontageLength = ActiveMontage->GetPlayLength();
    if (MontageLength <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float MontagePosition = AnimInstance->Montage_GetPosition(ActiveMontage);
    const float NormalizedTime = MontagePosition / MontageLength;
    const float RemainingTime = MontageLength - MontagePosition;

    // 최소 보장 시간을 넘기기 전에는 아무리 입력이 들어와도 현재 동작을 유지한다.
    if (NormalizedTime < ActiveNode.MinCommittedNormalizedTime)
    {
        return;
    }

    if (RemainingTime > ActiveNode.EarlyTransitionLeadTime)
    {
        return;
    }

    TryCommitQueuedComboTransition();
}

bool UMHGA_LongSwordCombo::TryCommitQueuedComboTransition()
{
    if (!CachedPlayer || !CachedWeapon || !CachedComboComponent || !bHasActiveNode)
    {
        return false;
    }

    const FGameplayTag QueuedPatternTag = CachedComboComponent->PeekBufferedInputPattern();
    if (!QueuedPatternTag.IsValid())
    {
        return false;
    }

    const FGameplayTag PreviousMoveTag = CachedComboComponent->GetCurrentMoveTag();
    const bool bCounterSuccess = CachedPlayer->HasLongSwordForesightCounterSuccess();
    const FMHLongSwordComboNode* NextNode = CachedComboComponent->SelectNextNode(QueuedPatternTag, bCounterSuccess);
    if (!NextNode)
    {
        UE_LOG(LogMHGALSCombo, Verbose, TEXT("조기 전환 가능한 다음 노드를 찾지 못했습니다. Current=%s Pattern=%s"), *PreviousMoveTag.ToString(), *QueuedPatternTag.ToString());
        return false;
    }

    CachedComboComponent->ConsumeBufferedInputPattern();
    CachedComboComponent->CommitMove(*NextNode);

    UAnimInstance* AnimInstance = CachedPlayer->GetMesh() ? CachedPlayer->GetMesh()->GetAnimInstance() : nullptr;
    if (AnimInstance && ActiveMontage)
    {
        // 현재 몽타주를 짧게 Blend Out 시킨 뒤 다음 몽타주를 이어붙인다.
        AnimInstance->Montage_Stop(ActiveNode.TransitionBlendOutTime, ActiveMontage);
    }

    bIgnoreMontageCallbacks = true;
    ClearTask();
    bIgnoreMontageCallbacks = false;

    UE_LOG(LogMHGALSCombo, Verbose, TEXT("조기 콤보 전환 실행. %s -> %s"), *PreviousMoveTag.ToString(), *NextNode->MoveTag.ToString());
    return PlayResolvedNode(*NextNode, PreviousMoveTag);
}

bool UMHGA_LongSwordCombo::IsDrawEntryMoveTag(const FGameplayTag& InMoveTag) const
{
    using namespace MHLongSwordGameplayTags;

    return InMoveTag == Move_LS_DrawOnly
        || InMoveTag == Move_LS_DrawAdvancingSlash
        || InMoveTag == Move_LS_DrawSpiritSlash1;
}

void UMHGA_LongSwordCombo::OnMontageCompleted()
{
    if (!CachedPlayer || !CachedWeapon || !CachedComboComponent)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    ClearTransitionPollingTimer();

    const FGameplayTag CompletedMoveTag = CachedComboComponent->GetCurrentMoveTag();
    CachedPlayer->HandleComboMontageStateTransition(false);

    if (CachedComboComponent->HasAcceptedBufferedInputPattern())
    {
        const FGameplayTag PatternTag = CachedComboComponent->ConsumeBufferedInputPattern();
        if (PatternTag.IsValid() && PlayNextMove(CachedPlayer, CachedWeapon, CachedComboComponent, PatternTag))
        {
            return;
        }
    }

    CachedComboComponent->ResetCombo();
    CachedPlayer->ClearLongSwordForesightCounterSuccess();
    CachedPlayer->TryStartAutoSheatheAfterLongSwordMove(CompletedMoveTag);
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMHGA_LongSwordCombo::OnMontageInterrupted()
{
    if (bIgnoreMontageCallbacks)
    {
        return;
    }

    ClearTransitionPollingTimer();

    if (CachedPlayer)
    {
        CachedPlayer->HandleComboMontageStateTransition(true);
        CachedPlayer->ClearLongSwordForesightCounterSuccess();
    }

    if (CachedComboComponent)
    {
        CachedComboComponent->ResetCombo();
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UMHGA_LongSwordCombo::ClearTask()
{
    if (MontageTask)
    {
        MontageTask->OnCompleted.RemoveDynamic(this, &UMHGA_LongSwordCombo::OnMontageCompleted);
        MontageTask->OnInterrupted.RemoveDynamic(this, &UMHGA_LongSwordCombo::OnMontageInterrupted);
        MontageTask->OnCancelled.RemoveDynamic(this, &UMHGA_LongSwordCombo::OnMontageInterrupted);
        MontageTask->EndTask();
        MontageTask = nullptr;
    }
}

void UMHGA_LongSwordCombo::ClearTransitionPollingTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TransitionPollingTimerHandle);
    }
}
