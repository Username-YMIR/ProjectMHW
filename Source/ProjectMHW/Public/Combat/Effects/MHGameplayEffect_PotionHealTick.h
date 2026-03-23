// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MHGameplayEffect_PotionHealTick.generated.h"

/**
 * 물약 지속 회복 1틱용 Instant GameplayEffect
 * - IncomingHeal += HealPerTick
 * 실제 Health 반영과 HealableHealth 차감은 AttributeSet에서 처리
 */
UCLASS()
class PROJECTMHW_API UMHGameplayEffect_PotionHealTick : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMHGameplayEffect_PotionHealTick();
};