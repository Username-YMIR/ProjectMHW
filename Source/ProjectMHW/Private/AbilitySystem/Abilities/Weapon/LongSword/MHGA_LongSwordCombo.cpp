#include "AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"
#include "Items/Instance/MHLongSwordInstance.h"
#include "Weapons/LongSword/MHLongSwordComboComponent.h"
#include "Weapons/LongSword/MHLongSwordComboGraph.h"

DEFINE_LOG_CATEGORY(LogMHGALSCombo);

UMHGA_LongSwordCombo::UMHGA_LongSwordCombo()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
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
    ClearTask();

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
        return false;
    }

    UAnimMontage* Montage = Node->Montage.IsNull() ? nullptr : Node->Montage.LoadSynchronous();
    if (!Montage)
    {
        UE_LOG(LogMHGALSCombo, Warning, TEXT("Montage is null. MoveTag=%s"), *Node->MoveTag.ToString());
        return false;
    }

    ComboComp->CommitMove(*Node);

    if (PreviousMoveTag == MHLongSwordGameplayTags::Move_LS_ForesightSlash)
    {
        Player->ClearLongSwordForesightCounterSuccess();
    }

    ClearTask();

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.0f, Node->SectionName, true);
    if (!MontageTask)
    {
        return false;
    }

    MontageTask->OnCompleted.AddDynamic(this, &UMHGA_LongSwordCombo::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UMHGA_LongSwordCombo::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UMHGA_LongSwordCombo::OnMontageInterrupted);

    MontageTask->ReadyForActivation();
    return true;
}

void UMHGA_LongSwordCombo::OnMontageCompleted()
{
    if (!CachedPlayer || !CachedWeapon || !CachedComboComponent)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

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
        MontageTask->EndTask();
        MontageTask = nullptr;
    }
}
