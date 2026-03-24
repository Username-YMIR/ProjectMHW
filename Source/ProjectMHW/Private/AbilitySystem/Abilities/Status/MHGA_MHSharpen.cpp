#include "AbilitySystem/Abilities/Status/MHGA_MHSharpen.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "MHGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogMHSharpenAbility, Log, All);

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
		UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] Activate failed: Player null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!SharpenMontage || !SharpenTickEffectClass)
	{
		UE_LOG(
			LogMHSharpenAbility,
			Warning,
			TEXT("[Sharpen] Activate failed: Assets invalid Montage=%s TickEffect=%s"),
			*GetNameSafe(SharpenMontage),
			*GetNameSafe(SharpenTickEffectClass));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] Activate failed: CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(
		LogMHSharpenAbility,
		Log,
		TEXT("[Sharpen] Activate Current=%.1f Max=%.1f Held=%d"),
		Player->GetCurrentSharpnessValue(),
		Player->GetMaxSharpnessValue(),
		Player->IsItemUseHeld() ? 1 : 0);

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
	(void)Payload;

	if (bSharpenStarted)
	{
		UE_LOG(LogMHSharpenAbility, Verbose, TEXT("[Sharpen] OnSharpenStart ignored: already started"));
		return;
	}

	bSharpenStarted = true;
	UE_LOG(LogMHSharpenAbility, Log, TEXT("[Sharpen] OnSharpenStart"));

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
		UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] Tick failed: Player or ASC null"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!Player->IsItemUseHeld())
	{
		UE_LOG(LogMHSharpenAbility, Log, TEXT("[Sharpen] Tick ended: ItemUse not held"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (Player->GetCurrentSharpnessValue() >= Player->GetMaxSharpnessValue())
	{
		UE_LOG(
			LogMHSharpenAbility,
			Log,
			TEXT("[Sharpen] Tick ended: Already full Current=%.1f Max=%.1f"),
			Player->GetCurrentSharpnessValue(),
			Player->GetMaxSharpnessValue());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(SharpenTickEffectClass, 1.f, Context);
	if (!Spec.IsValid() || !Spec.Data.IsValid())
	{
		UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] Tick failed: SharpenTick spec invalid"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	Player->RefreshSharpnessState();

	UE_LOG(
		LogMHSharpenAbility,
		Log,
		TEXT("[Sharpen] Tick applied Current=%.1f Max=%.1f"),
		Player->GetCurrentSharpnessValue(),
		Player->GetMaxSharpnessValue());

	if (Player->GetCurrentSharpnessValue() >= Player->GetMaxSharpnessValue())
	{
		UE_LOG(LogMHSharpenAbility, Log, TEXT("[Sharpen] Tick completed: Reached max sharpness"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_MHSharpen::HandleMontageCompleted()
{
	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!bSharpenStarted)
	{
		UE_LOG(LogMHSharpenAbility, Log, TEXT("[Sharpen] Montage completed before sharpen start"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (!Player || !Player->IsItemUseHeld() || Player->GetCurrentSharpnessValue() >= Player->GetMaxSharpnessValue())
	{
		UE_LOG(LogMHSharpenAbility, Log, TEXT("[Sharpen] Montage completed and ability ended"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_MHSharpen::HandleMontageInterrupted()
{
	UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] Montage interrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
