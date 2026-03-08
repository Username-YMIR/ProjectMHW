// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHEquipItemInstance.h"
#include "GameplayAbilitySpecHandle.h"
#include "Type/MHItemStructType.h" // 손승우 추가
#include "Type/MHWeaponAnimStructType.h" // 손승우 추가
#include "MHWeaponInstance.generated.h"

class USceneComponent; // 손승우 추가
class UAbilitySystemComponent; // 손승우 추가
class UGameplayAbility; // 손승우 추가

UCLASS()
class PROJECTMHW_API AMHWeaponInstance : public AMHEquipItemInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHWeaponInstance();


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* WeaponRoot; // 손승우 추가

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EMHWeaponType WeaponType = EMHWeaponType::None; // 손승우 추가

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	FMHWeaponAnimConfig WeaponAnimConfig; // 손승우 추가

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	TSubclassOf<UGameplayAbility> PrimaryAttackAbilityClass; // 손승우 추가

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle PrimaryAttackAbilityHandle; // 손승우 추가

	//TODO: 프로젝트용 구조체로 변경하기 _ 이건주 
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponData")
	// FHeroWeaponData HeroWeaponData;

public:
	FORCEINLINE USkeletalMeshComponent* GetWeaponMeshComponent() const { return WeaponMesh; } // 손승우 추가
	FORCEINLINE EMHWeaponType GetWeaponType() const { return WeaponType; } // 손승우 추가
	FORCEINLINE const FMHWeaponAnimConfig& GetWeaponAnimConfig() const { return WeaponAnimConfig; } // 손승우 추가

	// 등 파츠(검집/방패 등)
	virtual USkeletalMeshComponent* GetSheathMeshComponent() const { return nullptr; } // 손승우 추가

	// 무기 어빌리티 부여
	void GrantWeaponAbilities(UAbilitySystemComponent* ASC); // 손승우 추가

	// 무기 어빌리티 해제
	void ClearWeaponAbilities(UAbilitySystemComponent* ASC); // 손승우 추가

	// 기본 공격 어빌리티
	FORCEINLINE TSubclassOf<UGameplayAbility> GetPrimaryAttackAbilityClass() const { return PrimaryAttackAbilityClass; } // 손승우 추가
};
