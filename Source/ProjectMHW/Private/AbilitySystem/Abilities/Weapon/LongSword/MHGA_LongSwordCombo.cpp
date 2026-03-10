#include "AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/MHPlayerCharacter.h"
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

    AMHWeaponInstance* WeaponBase = Player->GetEquippedWeapon();
    AMHLongSwordInstance* Weapon = Cast<AMHLongSwordInstance>(WeaponBase);
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

    const FGameplayTag RequestedMoveTag = ComboComp->ConsumeBufferedRequestedMove();
    const EMHComboInputType InputType = ComboComp->ConsumeBufferedInput();
    if (!PlayNextMove(Player, Weapon, ComboComp, InputType == EMHComboInputType::None ? EMHComboInputType::Primary : InputType, RequestedMoveTag))
    {
        Player->HandleComboMontageStateTransition(true); //손승우 추가
        ComboComp->ResetCombo();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
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

bool UMHGA_LongSwordCombo::PlayNextMove(AMHPlayerCharacter* Player, AMHLongSwordInstance* Weapon, UMHLongSwordComboComponent* ComboComp, EMHComboInputType InputType, const FGameplayTag& RequestedMoveTag)
{
    if (!Player || !Weapon || !ComboComp)
    {
        return false;
    }

    const FMHLongSwordComboNode* Node = RequestedMoveTag.IsValid() ? ComboComp->GetComboGraph()->FindNode(RequestedMoveTag) : ComboComp->SelectNextNode(InputType);
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

    CachedPlayer->HandleComboMontageStateTransition(false); //손승우 추가

    if (CachedComboComponent->HasAcceptedBufferedInput())
    {
        const FGameplayTag RequestedMoveTag = CachedComboComponent->ConsumeBufferedRequestedMove();
        const EMHComboInputType InputType = CachedComboComponent->ConsumeBufferedInput();
        const EMHComboInputType UseInput = InputType == EMHComboInputType::None ? EMHComboInputType::Primary : InputType;

        if (PlayNextMove(CachedPlayer, CachedWeapon, CachedComboComponent, UseInput, RequestedMoveTag))
        {
            return;
        }
    }
    else if (CachedPlayer->TryStartAutoSheatheAfterLongSwordMove(CompletedMoveTag))
    {
        CachedComboComponent->ResetCombo();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    CachedComboComponent->ResetCombo();
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMHGA_LongSwordCombo::OnMontageInterrupted()
{
    if (CachedPlayer)
    {
        CachedPlayer->HandleComboMontageStateTransition(true); //손승우 추가
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
