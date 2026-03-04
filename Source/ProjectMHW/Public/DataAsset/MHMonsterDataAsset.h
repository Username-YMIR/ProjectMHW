// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/Monster/MonsterType/MHMonsterType.h"
#include "Engine/DataAsset.h"
#include "MHMonsterDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTMHW_API UMHMonsterDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category="Monster|Tag")
	FGameplayTagContainer MonsterTags ;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category="Monster|Tag")
	TArray<TSubclassOf<class UGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category="Monster|GAS")
	TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category="Monster|AI")
	TArray<FMonsterAbilityEntry> AbilityEntries;
	
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category="Monster|Phase")
	TArray<FPhaseAbilitySet> PhaseSet;
};
