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
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly);
	TObjectPtr<class UAnimMontage> Montage = nullptr;
	
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
