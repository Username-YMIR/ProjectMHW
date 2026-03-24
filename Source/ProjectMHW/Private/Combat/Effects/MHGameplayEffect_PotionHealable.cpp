// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Effects/MHGameplayEffect_PotionHealable.h"

#include "Combat/Attributes/MHHealthAttributeSet.h"

UMHGameplayEffect_PotionHealable::UMHGameplayEffect_PotionHealable()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMHHealthAttributeSet::GetHealableHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(150.0f));

	Modifiers.Add(ModifierInfo);
}
