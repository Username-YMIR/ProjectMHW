#include "AbilitySystem/Abilities/Status/MHGA_MHSharpen.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Combat/Attributes/MHPlayerAttributeSet.h"
#include "MHGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogMHSharpenAbility, Log, All);

UGA_MHSharpen::UGA_MHSharpen()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_MHSharpen::RequestExternalEndAbility(bool bInWasCancelled)
{
	if (!IsActive())
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bInWasCancelled);
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

	if (!SharpenMontage)
	{
		UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] Activate failed: SharpenMontage invalid"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!Player->CanStartSharpenItemUse())
	{
		UE_LOG(
			LogMHSharpenAbility,
			Log,
			TEXT("[Sharpen] Activate blocked Current=%.1f Max=%.1f Velocity=%.2f"),
			Player->GetCurrentSharpnessValue(),
			Player->GetMaxSharpnessValue(),
			Player->GetVelocity().Size2D()
		);
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
		TEXT("[Sharpen] Activate Current=%.1f Max=%.1f"),
		Player->GetCurrentSharpnessValue(),
		Player->GetMaxSharpnessValue());

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

	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Player || !ASC)
	{
		UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] OnSharpenStart failed: Player or ASC null"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bSharpenStarted = true;

	const float MaxSharpness = Player->GetMaxSharpnessValue();
	ASC->SetNumericAttributeBase(UMHPlayerAttributeSet::GetSharpnessAttribute(), MaxSharpness);
	Player->RefreshSharpnessState();

	UE_LOG(
		LogMHSharpenAbility,
		Log,
		TEXT("[Sharpen] OnSharpenStart restored to max Current=%.1f Max=%.1f"),
		Player->GetCurrentSharpnessValue(),
		Player->GetMaxSharpnessValue()
	);
}

void UGA_MHSharpen::TickSharpen()
{
}

void UGA_MHSharpen::HandleMontageCompleted()
{
	UE_LOG(LogMHSharpenAbility, Log, TEXT("[Sharpen] Montage completed"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MHSharpen::HandleMontageInterrupted()
{
	UE_LOG(LogMHSharpenAbility, Warning, TEXT("[Sharpen] Montage interrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
