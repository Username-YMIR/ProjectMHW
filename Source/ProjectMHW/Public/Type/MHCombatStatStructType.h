// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHCombatStatStructType.generated.h"

// 속성 타입
// 공격 속성, 속성 내성에 사용
UENUM(BlueprintType)
enum class EMHElementType : uint8
{
	None	UMETA(DisplayName="None"),
	Fire	UMETA(DisplayName="Fire"),
	Water	UMETA(DisplayName="Water"),
	Thunder UMETA(DisplayName="Thunder"),
	Ice		UMETA(DisplayName="Ice"),
	Dragon	UMETA(DisplayName="Dragon"),
};

// 전투 : 공격 스탯
// 공격력 : float
// 예리도 : float
// 회심률 : float 0.f~1.f
// 속성 : FGameplayTag::Element
USTRUCT(BlueprintType)
struct FMHAttackStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float AttackPower = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Sharpness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Affinity = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMHElementType Type = EMHElementType::None;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Element"))
	// FGameplayTag ElementTag;
};

// 전투 : 방어 스탯
USTRUCT(BlueprintType)
struct FMHDefenseStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float Defense = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float FireResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float WaterResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float ThunderResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float IceResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float DragonResist = 0.f;
	
	// // //속성 내성 키 :  FGameplayTag::Element 값 : float
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Element"))
	// TMap<FGameplayTag, float> ResistByTag;
};