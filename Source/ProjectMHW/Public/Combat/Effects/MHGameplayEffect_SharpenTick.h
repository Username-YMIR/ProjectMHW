// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MHGameplayEffect_SharpenTick.generated.h"

/**
 * 예리도 회복 1틱용 Instant GameplayEffect
 * - Sharpness += TickAmount
 */
UCLASS()
class PROJECTMHW_API UMHGameplayEffect_SharpenTick : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMHGameplayEffect_SharpenTick();
};