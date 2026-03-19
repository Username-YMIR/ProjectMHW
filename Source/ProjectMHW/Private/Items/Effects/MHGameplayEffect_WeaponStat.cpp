// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Effects/MHGameplayEffect_WeaponStat.h"

#include "Combat/Attributes/MHCombatAttributeSet.h"
#include "MHGameplayTags.h"

UMHGameplayEffect_WeaponStat::UMHGameplayEffect_WeaponStat()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// AttackPower += SetByCaller(Data.Weapon.AttackPower)
	{
		FGameplayModifierInfo ModifierInfo;
		ModifierInfo.Attribute = UMHCombatAttributeSet::GetAttackPowerAttribute();
		ModifierInfo.ModifierOp = EGameplayModOp::Additive;

		FSetByCallerFloat SetByCallerMagnitude;
		SetByCallerMagnitude.DataTag = MHGameplayTags::Data_Weapon_AttackPower;

		ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);
		Modifiers.Add(ModifierInfo);
	}

	// CriticalRate += SetByCaller(Data.Weapon.Affinity)
	{
		FGameplayModifierInfo ModifierInfo;
		ModifierInfo.Attribute = UMHCombatAttributeSet::GetCriticalRateAttribute();
		ModifierInfo.ModifierOp = EGameplayModOp::Additive;

		FSetByCallerFloat SetByCallerMagnitude;
		SetByCallerMagnitude.DataTag = MHGameplayTags::Data_Weapon_Affinity;

		ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);
		Modifiers.Add(ModifierInfo);
	}
}