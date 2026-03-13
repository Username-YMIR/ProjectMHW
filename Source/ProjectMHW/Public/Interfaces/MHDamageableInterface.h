// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Type/MHCombatStatStructType.h"
#include "UObject/Interface.h"
#include "MHDamageableInterface.generated.h"

// USTRUCT(BlueprintType)
// struct FMHHitAcknowledge
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite)
// 	bool bAcceptedHit = false;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite)
// 	bool bConsumeHitOnce = false;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite)
// 	bool bShouldStopAttackWindow = false;
// };
//
// // This class does not need to be modified.
UINTERFACE()
class UMHDamageableInterface : public UInterface
{
	GENERATED_BODY()
};
//
// /**
//  * 
//  */
class PROJECTMHW_API IMHDamageableInterface
{
	GENERATED_BODY()
//
// 	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
// public:
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
// 	void ApplyDamageContext(const FMHDamageContext& DamageContext);
// 	
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Combat")
// 	FMHHitAcknowledge ReceiveDamageSpec(
// 		AActor* SourceActor,
// 		AActor* SourceWeapon,
// 		FGameplayTag AttackTag,
// 		const FGameplayEffectSpecHandle& DamageSpecHandle,
// 		const FHitResult& HitResult);
};