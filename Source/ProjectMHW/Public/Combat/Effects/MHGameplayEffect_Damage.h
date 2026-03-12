// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MHGameplayEffect_Damage.generated.h"

/**
 * 기본 대미지 GameplayEffect
 * - Instant
 * - UMHDamageExecutionCalculation 실행
 */
UCLASS()
class PROJECTMHW_API UMHGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMHGameplayEffect_Damage();
};