#include "AbilitySystem/Abilities/Status/MHGA_Potion.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Combat/Attributes/MHHealthAttributeSet.h"
#include "MHGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogMHPotionAbility, Log, All);

UMHGA_Potion::UMHGA_Potion()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UMHGA_Potion::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Player)
	{
		UE_LOG(LogMHPotionAbility, Warning, TEXT("[Potion] ActivateAbility failed: invalid player"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!PotionMontage || !PotionHealableEffectClass || !PotionHealTickEffectClass)
	{
		UE_LOG(
			LogMHPotionAbility,
			Warning,
			TEXT("[Potion] ActivateAbility failed: missing asset montage=%d healableGE=%d tickGE=%d"),
			PotionMontage != nullptr,
			PotionHealableEffectClass != nullptr,
			PotionHealTickEffectClass != nullptr
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentHealth = Player->GetCurrentHealthValue();
	const float MaxHealth = Player->GetMaxHealthValue();
	if (CurrentHealth >= MaxHealth)
	{
		UE_LOG(
			LogMHPotionAbility,
			Log,
			TEXT("[Potion] ActivateAbility cancelled: already full health (HP=%.1f/%.1f)"),
			CurrentHealth,
			MaxHealth
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogMHPotionAbility, Warning, TEXT("[Potion] ActivateAbility failed: CommitAbility returned false"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(
		LogMHPotionAbility,
		Log,
		TEXT("[Potion] ActivateAbility success: HP=%.1f/%.1f Healable=%.1f"),
		CurrentHealth,
		MaxHealth,
		Player->GetCurrentHealableHealthValue()
	);

	bDrinkTriggered = false;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		PotionMontage
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageInterrupted);
		MontageTask->ReadyForActivation();
	}

	DrinkEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		MHGameplayTags::Event_Item_Drink
	);

	if (DrinkEventTask)
	{
		DrinkEventTask->EventReceived.AddDynamic(this, &ThisClass::OnDrink);
		DrinkEventTask->ReadyForActivation();
	}
}

void UMHGA_Potion::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HealTickTimerHandle);
	}

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (DrinkEventTask)
	{
		DrinkEventTask->EndTask();
		DrinkEventTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMHGA_Potion::OnDrink(FGameplayEventData Payload)
{
	if (bDrinkTriggered)
	{
		UE_LOG(LogMHPotionAbility, Verbose, TEXT("[Potion] OnDrink ignored: already triggered"));
		return;
	}

	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Player || !ASC)
	{
		UE_LOG(LogMHPotionAbility, Warning, TEXT("[Potion] OnDrink failed: player or ASC invalid"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const float CurrentHealth = Player->GetCurrentHealthValue();
	const float MaxHealth = Player->GetMaxHealthValue();
	const float MissingHealth = FMath::Max(0.f, MaxHealth - CurrentHealth);
	if (MissingHealth <= 0.f)
	{
		UE_LOG(
			LogMHPotionAbility,
			Log,
			TEXT("[Potion] OnDrink cancelled: no missing health (HP=%.1f/%.1f)"),
			CurrentHealth,
			MaxHealth
		);
		ASC->SetNumericAttributeBase(UMHHealthAttributeSet::GetHealableHealthAttribute(), 0.f);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle HealableSpec = ASC->MakeOutgoingSpec(PotionHealableEffectClass, 1.f, Context);
	if (!HealableSpec.IsValid() || !HealableSpec.Data.IsValid())
	{
		UE_LOG(LogMHPotionAbility, Warning, TEXT("[Potion] OnDrink failed: invalid healable spec"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ASC->ApplyGameplayEffectSpecToSelf(*HealableSpec.Data.Get());
	const float ClampedHealableHealth = FMath::Min(Player->GetCurrentHealableHealthValue(), MissingHealth);
	ASC->SetNumericAttributeBase(UMHHealthAttributeSet::GetHealableHealthAttribute(), ClampedHealableHealth);
	bDrinkTriggered = true;

	UE_LOG(
		LogMHPotionAbility,
		Log,
		TEXT("[Potion] OnDrink: HP=%.1f/%.1f Missing=%.1f AppliedHealable=%.1f"),
		CurrentHealth,
		MaxHealth,
		MissingHealth,
		ClampedHealableHealth
	);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HealTickTimerHandle,
			this,
			&ThisClass::TickHeal,
			0.2f,
			true
		);
	}
}

void UMHGA_Potion::TickHeal()
{
	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Player || !ASC)
	{
		UE_LOG(LogMHPotionAbility, Warning, TEXT("[Potion] TickHeal failed: player or ASC invalid"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (Player->GetCurrentHealthValue() >= Player->GetMaxHealthValue())
	{
		ASC->SetNumericAttributeBase(UMHHealthAttributeSet::GetHealableHealthAttribute(), 0.f);
		UE_LOG(
			LogMHPotionAbility,
			Log,
			TEXT("[Potion] TickHeal finished: full health reached (HP=%.1f/%.1f)"),
			Player->GetCurrentHealthValue(),
			Player->GetMaxHealthValue()
		);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (Player->GetCurrentHealableHealthValue() <= 0.f)
	{
		UE_LOG(
			LogMHPotionAbility,
			Log,
			TEXT("[Potion] TickHeal finished: no healable health left (HP=%.1f/%.1f)"),
			Player->GetCurrentHealthValue(),
			Player->GetMaxHealthValue()
		);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle HealSpec = ASC->MakeOutgoingSpec(PotionHealTickEffectClass, 1.f, Context);
	if (!HealSpec.IsValid() || !HealSpec.Data.IsValid())
	{
		UE_LOG(LogMHPotionAbility, Warning, TEXT("[Potion] TickHeal failed: invalid tick heal spec"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ASC->ApplyGameplayEffectSpecToSelf(*HealSpec.Data.Get());

	UE_LOG(
		LogMHPotionAbility,
		Verbose,
		TEXT("[Potion] TickHeal applied: HP=%.1f/%.1f Healable=%.1f"),
		Player->GetCurrentHealthValue(),
		Player->GetMaxHealthValue(),
		Player->GetCurrentHealableHealthValue()
	);

	if (Player->GetCurrentHealableHealthValue() <= 0.f)
	{
		UE_LOG(LogMHPotionAbility, Log, TEXT("[Potion] TickHeal finished after apply: healable depleted"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UMHGA_Potion::HandleMontageCompleted()
{
	AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!bDrinkTriggered)
	{
		UE_LOG(LogMHPotionAbility, Log, TEXT("[Potion] Montage completed before Drink notify"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (!Player || Player->GetCurrentHealableHealthValue() <= 0.f)
	{
		UE_LOG(LogMHPotionAbility, Log, TEXT("[Potion] Montage completed and potion work is already done"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UMHGA_Potion::HandleMontageInterrupted()
{
	UE_LOG(LogMHPotionAbility, Warning, TEXT("[Potion] Montage interrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
