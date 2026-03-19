// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MHGameplayEffect_WeaponStat.generated.h"

/**
 * 무기 장착 중 유지되는 스탯 버프 GE
 * - Infinite
 * - AttackPower, CriticalRate를 SetByCaller로 받는다
 */
UCLASS()
class PROJECTMHW_API UMHGameplayEffect_WeaponStat : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMHGameplayEffect_WeaponStat();
};