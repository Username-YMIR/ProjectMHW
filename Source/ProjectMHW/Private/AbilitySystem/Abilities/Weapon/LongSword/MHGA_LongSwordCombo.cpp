#include "AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h"

#include "AbilitySystemComponent.h"
#include "MHGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Engine/World.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"
#include "Items/Instance/MHLongSwordInstance.h"
#include "TimerManager.h"
#include "Combat/Attributes/MHCombatAttributeSet.h"
#include "Combat/Effects/MHGameplayEffect_Damage.h"
#include "Weapons/LongSword/MHLongSwordComboComponent.h"
#include "Weapons/LongSword/MHLongSwordComboGraph.h"

DEFINE_LOG_CATEGORY(LogMHGALSCombo);

UMHGA_LongSwordCombo::UMHGA_LongSwordCombo()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    //===================================
    // 대미지 시스템 _이건주
    DamageEffectClass = UMHGameplayEffect_Damage::StaticClass();

    // 프로젝트에서 이미 사용 중인 SetByCaller 태그 기준
    PhysicalDamageDataTag = MHGameplayTags::Data_Damage_Physical;

    // Element 통합 태그를 쓰지 않는 구조라면 이 부분은 프로젝트 규칙에 맞게 교체
    // 예: Fire / Water / Thunder 등 개별 태그 처리
    ElementDamageDataTag = FGameplayTag();

    // 실제 프로젝트 AttributeSet에 맞게 에디터 또는 C++에서 지정
    SourcePhysicalAttackAttribute = UMHCombatAttributeSet::GetAttackPowerAttribute(); // 
    SourceElementAttackAttribute = FGameplayAttribute(); // TODO: 현재 어트리뷰트에 속성값이 없음. 추후 수정
    //===================================
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
    
    //공격 초기 데이터로 리셋_이건주
    ResetMeleeWeaponAttack();

    ActiveMontage = nullptr;
    bHasActiveNode = false;
    bIgnoreMontageCallbacks = false;

    CachedComboComponent = nullptr;
    CachedPlayer = nullptr;
    CachedWeapon = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

/** 외부 시스템에서 현재 콤보 어빌리티를 조기 종료할 때 호출_이건주 */
void UMHGA_LongSwordCombo::RequestExternalEndAbility(bool bInWasCancelled)
{
    if (!IsActive())
    {
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bInWasCancelled);
}

/** 현재 콤보 노드 기준 DamageSpec을 생성해서 무기 인스턴스에 전달 _이건주*/
bool UMHGA_LongSwordCombo::PushDamageSpecToWeapon(const FMHLongSwordComboNode& InNode)
{
    UE_LOG(LogMHGALSCombo, Warning, TEXT("PushDamageSpecToWeapon"));
    if (!CachedWeapon)
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("CachedWeapon is not Valid"));
        return false;
    }

    FGameplayEffectSpecHandle DamageSpecHandle;
    FGameplayTag AttackTag;

    if (!BuildDamageSpecForNode(InNode, DamageSpecHandle, AttackTag))
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("BuildDamageSpecForNode failed. Move=%s"), *InNode.MoveTag.ToString());
        return false;
    }

    CachedWeapon->SetCurrentDamageSpec(DamageSpecHandle);
    CachedWeapon->SetCurrentAttackTag(AttackTag);

    return true;
}

/** 무기 인스턴스에 전달된 현재 공격 데이터를 초기화 */
void UMHGA_LongSwordCombo::ClearCurrentAttackDataFromWeapon()
{
    if (!CachedWeapon)
    {
        return;
    }

    CachedWeapon->ClearCurrentAttackData();
}

/** Source ASC의 Attribute 값을 읽어 현재 노드 기준 DamageSpec 생성 _이건주*/
bool UMHGA_LongSwordCombo::BuildDamageSpecForNode(
    const FMHLongSwordComboNode& InNode,
    FGameplayEffectSpecHandle& OutSpecHandle,
    FGameplayTag& OutAttackTag) const
{
    OutSpecHandle = FGameplayEffectSpecHandle();
    OutAttackTag = FGameplayTag();

    if (!DamageEffectClass)
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("DamageEffectClass is null."));
        return false;
    }

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(SourceASC))
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("SourceASC is invalid."));
        return false;
    }

    FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();

    if (CachedPlayer)
    {
        EffectContext.AddInstigator(CachedPlayer.Get(), CachedPlayer.Get());
    }

    if (CachedWeapon)
    {
        EffectContext.AddSourceObject(CachedWeapon.Get());
    }

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("Failed to create outgoing spec. Move=%s"), *InNode.MoveTag.ToString());
        return false;
    }

    const float SourcePhysicalAttack = GetSourceAttributeMagnitude(SourcePhysicalAttackAttribute);
    const float SourceElementAttack = GetSourceAttributeMagnitude(SourceElementAttackAttribute);

    float NodeDamageMultiplier = 1.0f;
    if (CachedPlayer)
    {
        NodeDamageMultiplier = CachedPlayer->ResolveLongSwordDamageMultiplier(InNode.MoveTag);
    }

    const float FinalPhysicalDamage = SourcePhysicalAttack * GlobalPhysicalScale * NodeDamageMultiplier;
    const float FinalElementDamage = SourceElementAttack * GlobalElementScale * NodeDamageMultiplier;
    
    // 대미지 로그 _이건주
    UE_LOG(LogTemp, Warning, TEXT("[DamageSpec Build] SourceAP=%.2f SourceElement=%.2f Move=%s FinalPhysical=%.2f FinalElement=%.2f"),
    SourcePhysicalAttack,
    SourceElementAttack,
    *InNode.MoveTag.ToString(),
    FinalPhysicalDamage,
    FinalElementDamage);

    if (PhysicalDamageDataTag.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(PhysicalDamageDataTag, FinalPhysicalDamage);
    }

    if (ElementDamageDataTag.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(ElementDamageDataTag, FinalElementDamage);
    }

    // 현재 설계에서는 MoveTag를 기술 식별용 AttackTag로 사용
    OutAttackTag = InNode.MoveTag;
    OutSpecHandle = SpecHandle;

    UE_LOG(
        LogMHGALSCombo,
        Verbose,
        TEXT("DamageSpec built. Move=%s Physical=%.2f Element=%.2f NodeMultiplier=%.2f"),
        *InNode.MoveTag.ToString(),
        FinalPhysicalDamage,
        FinalElementDamage,
        NodeDamageMultiplier
    );

    return true;
}

/** 공격 원본 스탯을 ASC에서 읽는다 _이건주*/
float UMHGA_LongSwordCombo::GetSourceAttributeMagnitude(const FGameplayAttribute& InAttribute) const
{
    if (!InAttribute.IsValid())
    {
        return 0.0f;
    }

    const UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(SourceASC))
    {
        return 0.0f;
    }

    return SourceASC->GetNumericAttribute(InAttribute);
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

    if (!Player->CanStartLongSwordMove(Node->MoveTag))
    {
        UE_LOG(LogMHGALSCombo, Verbose, TEXT("기인 게이지가 부족해 기술을 시작할 수 없습니다. Move=%s"), *Node->MoveTag.ToString());
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

    CachedPlayer->ClearLongSwordCounterSuccessFlagsForMoveExit(InPreviousMoveTag);

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
    
    // 현재 콤보 노드 기준 공격 Spec을 무기 인스턴스에 전달한다.
    if (!PushDamageSpecToWeapon(InNode))
    {
        return false;
    }

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
        UE_LOG(LogMHGALSCombo, Verbose, TEXT("조기 전환 가능한 다음 노드를 찾지 못했습니다. Current=%s Pattern=%s"),
            *PreviousMoveTag.ToString(), *QueuedPatternTag.ToString());
        return false;
    }

    // 1) 다음 몽타주가 실제로 재생 가능한지 먼저 확인
    UAnimMontage* NextMontage = NextNode->Montage.IsNull() ? nullptr : NextNode->Montage.LoadSynchronous();
    NextMontage = CachedPlayer->ResolveLongSwordMoveMontageOverride(NextNode->MoveTag, NextMontage);
    if (!NextMontage)
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("조기 전환 실패: 다음 몽타주가 없습니다. MoveTag=%s"), *NextNode->MoveTag.ToString());
        return false;
    }

    // 값 보존
    if (!CachedPlayer->CanStartLongSwordMove(NextNode->MoveTag))
    {
        UE_LOG(LogMHGALSCombo, Verbose, TEXT("기인 게이지가 부족해 조기 전환을 시작할 수 없습니다. Move=%s"), *NextNode->MoveTag.ToString());
        return false;
    }

    const FMHLongSwordComboNode NextNodeCopy = *NextNode;

    // 2) 인터럽트 콜백 무시를 먼저 켠다
    bIgnoreMontageCallbacks = true;

    // 3) 현재 몽타주 정지
    if (UAnimInstance* AnimInstance = CachedPlayer->GetMesh() ? CachedPlayer->GetMesh()->GetAnimInstance() : nullptr)
    {
        if (ActiveMontage)
        {
            AnimInstance->Montage_Stop(ActiveNode.TransitionBlendOutTime, ActiveMontage);
        }
    }

    // 4) 기존 task 정리
    ClearTask();

    // 5) 이제 새 입력/이동 커밋
    CachedComboComponent->ConsumeBufferedInputPattern();
    CachedComboComponent->CommitMove(NextNodeCopy);

    // 6) 새 몽타주 재생 전 콜백 무시 해제
    bIgnoreMontageCallbacks = false;

    UE_LOG(LogMHGALSCombo, Verbose, TEXT("조기 콤보 전환 실행. %s -> %s"),
        *PreviousMoveTag.ToString(), *NextNodeCopy.MoveTag.ToString());

    return PlayResolvedNode(NextNodeCopy, PreviousMoveTag);
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
    
    ResetMeleeWeaponAttack();

    CachedComboComponent->ResetCombo();
    CachedPlayer->ClearAllLongSwordCounterSuccessFlags();
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
        CachedPlayer->ClearAllLongSwordCounterSuccessFlags();
    }

    if (CachedComboComponent)
    {
        CachedComboComponent->ResetCombo();
    }
    
    ResetMeleeWeaponAttack();

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

void UMHGA_LongSwordCombo::ResetMeleeWeaponAttack()
{
    if (!CachedWeapon)
    {
        return;
    }
    CachedWeapon->ResetMeleeAttack();
}
