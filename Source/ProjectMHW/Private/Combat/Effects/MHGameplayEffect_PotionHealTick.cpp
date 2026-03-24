// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Effects/MHGameplayEffect_PotionHealTick.h"

#include "Combat/Attributes/MHHealthAttributeSet.h"

UMHGameplayEffect_PotionHealTick::UMHGameplayEffect_PotionHealTick()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMHHealthAttributeSet::GetIncomingHealAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(10.0f));

	Modifiers.Add(ModifierInfo);
}
