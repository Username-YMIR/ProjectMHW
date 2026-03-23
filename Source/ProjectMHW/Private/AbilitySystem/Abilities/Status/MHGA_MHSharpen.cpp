#include "AbilitySystem/Abilities/Status/MHGA_MHSharpen.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "MHGameplayTags.h"

UGA_MHSharpen::UGA_MHSharpen()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_MHSharpen::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Player)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!SharpenMontage || !SharpenTickEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bSharpenStarted = false;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SharpenMontage
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageInterrupted);
		MontageTask->ReadyForActivation();
	}

	SharpenEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		MHGameplayTags::Event_Item_SharpenStart
	);

	if (SharpenEventTask)
	{
		SharpenEventTask->EventReceived.AddDynamic(this, &ThisClass::OnSharpenStart);
		SharpenEventTask->ReadyForActivation();
	}
}

void UGA_MHSharpen::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SharpenTickTimerHandle);
	}

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (SharpenEventTask)
	{
		SharpenEventTask->EndTask();
		SharpenEventTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MHSharpen::OnSharpenStart(FGameplayEventData Payload)
{
	if (bSharpenStarted)
	{
		return;
	}

	bSharpenStarted = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SharpenTickTimerHandle,
			this,
			&ThisClass::TickSharpen,
			0.2f,
			true
		);
	}
}

void UGA_MHSharpen::TickSharpen()
{
	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Player || !ASC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!Player->IsItemUseHeld())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (Player->GetCurrentSharpnessValue() >= Player->GetMaxSharpnessValue())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(SharpenTickEffectClass, 1.f, Context);
	if (!Spec.IsValid() || !Spec.Data.IsValid())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (Player->GetCurrentSharpnessValue() >= Player->GetMaxSharpnessValue())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_MHSharpen::HandleMontageCompleted()
{
	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!bSharpenStarted)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (!Player || !Player->IsItemUseHeld() || Player->GetCurrentSharpnessValue() >= Player->GetMaxSharpnessValue())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_MHSharpen::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
