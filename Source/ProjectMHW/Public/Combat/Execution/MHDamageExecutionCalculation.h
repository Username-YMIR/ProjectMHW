// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MHDamageExecutionCalculation.generated.h"

/**
 * 몬헌 스타일 기본 대미지 계산 Execution
 * - Source: AttackPower, CriticalRate, SharpnessModifier
 * - Target: Defense, ElementResists
 * - Output: IncomingDamage
 */
UCLASS()
class PROJECTMHW_API UMHDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UMHDamageExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;

protected:
	float CalculatePhysicalDamage(
		float InBasePhysicalDamage,
		float InAttackPower,
		float InSharpnessModifier,
		float InDefense
	) const;

	float CalculateElementDamage(
		float InBaseElementDamage,
		float InResist
	) const;

	float ApplyCritical(
		float InDamage,
		float InCriticalRate
	) const;
};