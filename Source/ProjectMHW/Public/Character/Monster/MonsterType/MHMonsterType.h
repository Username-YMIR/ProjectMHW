// 제작자 : 허혁
// 제작일 : 2026-03-08
// 수정자 : 허혁
// 수정일 : 2026-03-13


#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MHMonsterType.generated.h"


USTRUCT(BlueprintType)
struct FMonsterAbilityEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FGameplayTag AbilityTag;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<class UGameplayAbility> AbilityClass;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> Montage = nullptr;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly ,Category="Cooldown" ,meta=(ClampMin ="0.0"))
	float CooldownSeconds = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PhysicalDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRange = 220.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HitRadius = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName HitSocketName = NAME_None;
};

USTRUCT(BlueprintType)
struct FPhaseAbilitySet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float Weight = 1.f;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly)
	FGameplayTag PhaseTag;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly)
	TArray<FMonsterAbilityEntry> AbilityEntries;
	
};
