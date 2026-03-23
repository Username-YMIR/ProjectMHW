// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MHGameplayEffect_PotionHealable.generated.h"

/**
 * 물약 사용 시 회복 예정 체력(HealableHealth)을 즉시 추가하는 Instant GameplayEffect
 * - HealableHealth += PotionAmount
 */
UCLASS()
class PROJECTMHW_API UMHGameplayEffect_PotionHealable : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMHGameplayEffect_PotionHealable();
};