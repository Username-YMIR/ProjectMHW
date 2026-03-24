// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Effects/MHGameplayEffect_SharpenTick.h"

#include "Combat/Attributes/MHPlayerAttributeSet.h"

UMHGameplayEffect_SharpenTick::UMHGameplayEffect_SharpenTick()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UMHPlayerAttributeSet::GetSharpnessAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(10.0f));

	Modifiers.Add(ModifierInfo);
}
