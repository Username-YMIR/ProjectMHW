// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHItemInstanceBase.h"
#include "GameplayAbilitySpecHandle.h"

#include "MHEquipItemInstance.generated.h"



UCLASS()
class PROJECTMHW_API AMHEquipItemInstance : public AMHItemInstanceBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHEquipItemInstance();

public:		
	UFUNCTION(BlueprintCallable)
	void AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles);
		
	TArray<FGameplayAbilitySpecHandle> GetGrantedAbilitySpecHandles() const;
		
private:
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;
};
