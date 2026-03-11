// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Effects/MHGameplayEffect_Damage.h"

#include "Combat/Execution/MHDamageExecutionCalculation.h"

UMHGameplayEffect_Damage::UMHGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecutionDef;
	ExecutionDef.CalculationClass = UMHDamageExecutionCalculation::StaticClass();

	Executions.Add(ExecutionDef);
}