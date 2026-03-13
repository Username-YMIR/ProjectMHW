// Fill out your copyright notice in the Description page of Project Settings.
// 제작자 : 이건주
// 제작일 : 2026-03-05
// 수정일 : 2026-03-05 
#pragma once
// 제작자 : 이건주
// 제작일 : 2026-03-09
// 수정일 : 2026-03-09 
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHCombatStatStructType.generated.h"

/////// Legacy
// // 속성 타입
// // 공격 속성, 속성 내성에 사용
// UENUM(BlueprintType)
// enum class EMHElementType : uint8
// {
// 	None	UMETA(DisplayName="None"),
// 	Fire	UMETA(DisplayName="Fire"),
// 	Water	UMETA(DisplayName="Water"),
// 	Thunder UMETA(DisplayName="Thunder"),
// 	Ice		UMETA(DisplayName="Ice"),
// 	Dragon	UMETA(DisplayName="Dragon"),
// };

class UMHGameplayEffect_Damage;

UENUM(BlueprintType)
enum class EMHSharpnessColor : uint8
{
	Red,
	Orange,
	Yellow,
	Green,
	Blue,
	White,
};

USTRUCT(BlueprintType)
struct FMHSharpnessData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sharpness", meta=(ClampMin="0", UIMin="0"))
	float Red = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sharpness", meta=(ClampMin="0", UIMin="0"))
	float Orange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sharpness", meta=(ClampMin="0", UIMin="0"))
	float Yellow = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sharpness", meta=(ClampMin="0", UIMin="0"))
	float Green = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sharpness", meta=(ClampMin="0", UIMin="0"))
	float Blue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sharpness", meta=(ClampMin="0", UIMin="0"))
	float White = 0.f;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0", UIMin="0"))
	float Affinity = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Element"))
	FGameplayTag AttackElementTag;
};

// 전투 : 방어 스탯
USTRUCT(BlueprintType)
struct FMHDefenseStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float DefensePower = 0.f;
	
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
};

//---------------------
//대미지 전달 구조체_이건주
//---------------------
USTRUCT(BlueprintType)
struct FMHDamageContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> CauserActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 ComboIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	FName HitBoneName = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageTypeTag;
};

// 공격 결과 리턴 값
UENUM(BlueprintType)
enum class EMHHitResult : uint8
{
	None        UMETA(DisplayName="None"),
	Hit         UMETA(DisplayName="Hit"),
	Deflected   UMETA(DisplayName="Deflected"),
	Blocked     UMETA(DisplayName="Blocked"),
	Ignored     UMETA(DisplayName="Ignored"),
	Invincible  UMETA(DisplayName="Invincible")
};

USTRUCT(BlueprintType)
struct FMHMeleeAttackPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	TSubclassOf<UMHGameplayEffect_Damage> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	float PhysicalDamage = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	float ElementDamage = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	FGameplayTag AttakElementTag;
};

