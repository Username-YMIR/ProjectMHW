// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Attributes/MHCombatAttributeSet.h"
#include "Net/UnrealNetwork.h"

UMHCombatAttributeSet::UMHCombatAttributeSet()
{
	InitAttackPower(0.f);
	InitDefense(0.f);
	InitCriticalRate(0.f);
	InitSharpnessModifier(1.f);
}

void UMHCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetAttackPowerAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetCriticalRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if (Attribute == GetSharpnessModifierAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UMHCombatAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHCombatAttributeSet, AttackPower, OldValue);
}

void UMHCombatAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHCombatAttributeSet, Defense, OldValue);
}

void UMHCombatAttributeSet::OnRep_CriticalRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHCombatAttributeSet, CriticalRate, OldValue);
}

void UMHCombatAttributeSet::OnRep_SharpnessModifier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHCombatAttributeSet, SharpnessModifier, OldValue);
}

void UMHCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMHCombatAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHCombatAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHCombatAttributeSet, CriticalRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHCombatAttributeSet, SharpnessModifier, COND_None, REPNOTIFY_Always);
}