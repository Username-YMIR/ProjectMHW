#include "AbilitySystem/Abilities/Weapon/GreatSword/MHGA_GreatSwordAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Combat/Attributes/MHCombatAttributeSet.h"
#include "Combat/Data/MHAttackMetaTypes.h"
#include "Combat/Effects/MHGameplayEffect_Damage.h"
#include "Items/Instance/MHGreatSwordInstance.h"
#include "MHGameplayTags.h"
#include "Weapons/GreatSword/MHGreatSwordActionComponent.h"

DEFINE_LOG_CATEGORY(LogMHGAGreatSwordAttack);

UMHGA_GreatSwordAttack::UMHGA_GreatSwordAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    DamageEffectClass = UMHGameplayEffect_Damage::StaticClass();
    SourcePhysicalAttackAttribute = UMHCombatAttributeSet::GetAttackPowerAttribute();
    PhysicalDamageDataTag = MHGameplayTags::Data_Damage_Physical;
}

void UMHGA_GreatSwordAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    CachedPlayer = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!CachedPlayer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CachedWeapon = Cast<AMHGreatSwordInstance>(CachedPlayer->GetEquippedWeapon());
    if (!CachedWeapon)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UMHGreatSwordActionComponent* ActionComponent = CachedWeapon->GetActionComponent();
    if (!ActionComponent)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ActiveMoveTag = ActionComponent->ConsumePendingMoveTag();
    if (!ActiveMoveTag.IsValid())
    {
        UE_LOG(LogMHGAGreatSwordAttack, Verbose, TEXT("대기 중인 대검 기술이 없어 어빌리티를 종료합니다."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAnimMontage* MoveMontage = ActionComponent->ResolveMontageForMove(ActiveMoveTag);
    if (!MoveMontage)
    {
        UE_LOG(LogMHGAGreatSwordAttack, Warning, TEXT("대검 몽타주를 찾지 못했습니다. Move=%s"), *ActiveMoveTag.ToString());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!PushCurrentAttackDataToWeapon(ActiveMoveTag))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ActionComponent->CommitExecutedMove(ActiveMoveTag);

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MoveMontage, 1.0f, NAME_None, true);
    if (!MontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    MontageTask->OnCompleted.AddDynamic(this, &UMHGA_GreatSwordAttack::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UMHGA_GreatSwordAttack::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UMHGA_GreatSwordAttack::OnMontageInterrupted);
    MontageTask->ReadyForActivation();
}

void UMHGA_GreatSwordAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    ClearTask();
    ClearCurrentAttackDataFromWeapon();
    FinalizeGreatSwordActionState();
    ActiveMoveTag = FGameplayTag();
    CachedWeapon = nullptr;
    CachedPlayer = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UMHGA_GreatSwordAttack::BuildDamageSpecForMove(const FGameplayTag& InMoveTag, FGameplayEffectSpecHandle& OutSpecHandle) const
{
    OutSpecHandle = FGameplayEffectSpecHandle();

    if (!DamageEffectClass)
    {
        return false;
    }

    const UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(SourceASC) || !CachedWeapon)
    {
        return false;
    }

    FMHAttackMetaRow AttackMetaRow;
    if (!CachedWeapon->GetActionComponent() || !CachedWeapon->GetActionComponent()->FindAttackMetaRow(InMoveTag, AttackMetaRow))
    {
        UE_LOG(LogMHGAGreatSwordAttack, Warning, TEXT("대검 공격 메타를 찾지 못했습니다. Move=%s"), *InMoveTag.ToString());
        return false;
    }

    FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
    if (CachedPlayer)
    {
        EffectContext.AddInstigator(CachedPlayer.Get(), CachedPlayer.Get());
    }

    EffectContext.AddSourceObject(CachedWeapon.Get());

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        return false;
    }

    const float SourcePhysicalAttack = SourceASC->GetNumericAttribute(SourcePhysicalAttackAttribute);
    const float FinalPhysicalDamage = SourcePhysicalAttack * GlobalPhysicalScale * AttackMetaRow.DamageMultiplier;

    SpecHandle.Data->SetSetByCallerMagnitude(PhysicalDamageDataTag, FinalPhysicalDamage);
    OutSpecHandle = SpecHandle;
    return true;
}

bool UMHGA_GreatSwordAttack::PushCurrentAttackDataToWeapon(const FGameplayTag& InMoveTag)
{
    if (!CachedWeapon)
    {
        return false;
    }

    FGameplayEffectSpecHandle DamageSpecHandle;
    if (!BuildDamageSpecForMove(InMoveTag, DamageSpecHandle))
    {
        return false;
    }

    CachedWeapon->SetCurrentAttackTag(InMoveTag);
    CachedWeapon->SetCurrentDamageSpec(DamageSpecHandle);
    return true;
}

void UMHGA_GreatSwordAttack::ClearCurrentAttackDataFromWeapon()
{
    if (!CachedWeapon)
    {
        return;
    }

    CachedWeapon->ClearCurrentAttackData();
}

void UMHGA_GreatSwordAttack::FinalizeGreatSwordActionState()
{
    if (!CachedWeapon || !CachedWeapon->GetActionComponent())
    {
        return;
    }

    CachedWeapon->GetActionComponent()->NotifyActionFinished();
}

void UMHGA_GreatSwordAttack::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMHGA_GreatSwordAttack::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UMHGA_GreatSwordAttack::ClearTask()
{
    if (!MontageTask)
    {
        return;
    }

    MontageTask->EndTask();
    MontageTask = nullptr;
}
