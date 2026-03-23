// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Attributes/MHPlayerAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UMHPlayerAttributeSet::UMHPlayerAttributeSet()
{
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitSharpness(0.f);
	InitMaxSharpness(0.f);
}

void UMHPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetMaxSharpnessAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetSharpnessAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UMHPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		SetMaxStamina(FMath::Max(0.f, GetMaxStamina()));
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetSharpnessAttribute())
	{
		SetSharpness(FMath::Max(0.f, GetSharpness()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxSharpnessAttribute())
	{
		SetMaxSharpness(FMath::Max(0.f, GetMaxSharpness()));
	}
}

void UMHPlayerAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHPlayerAttributeSet, Stamina, OldValue);
}

void UMHPlayerAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHPlayerAttributeSet, MaxStamina, OldValue);
}

void UMHPlayerAttributeSet::OnRep_Sharpness(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHPlayerAttributeSet, Sharpness, OldValue);

}

void UMHPlayerAttributeSet::OnRep_MaxSharpness(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHPlayerAttributeSet, MaxSharpness, OldValue);

}

void UMHPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMHPlayerAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHPlayerAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UMHPlayerAttributeSet, Sharpness, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHPlayerAttributeSet, MaxSharpness, COND_None, REPNOTIFY_Always);

}