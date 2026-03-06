// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHWeaponInstance.h"
#include "MHMeleeWeaponInstance.generated.h"

DECLARE_DELEGATE_OneParam(FOnTargetInteractedDelegate, AActor*)

class UBoxComponent;

UCLASS()
class PROJECTMHW_API AMHMeleeWeaponInstance : public AMHWeaponInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHMeleeWeaponInstance();
	
	FOnTargetInteractedDelegate OnWeaponHitTarget;
	FOnTargetInteractedDelegate OnWeaponPulledFromTarget;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UBoxComponent* WeaponCollisionBox;
	
public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const{return WeaponCollisionBox;}

	UFUNCTION()
	virtual void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
   
	UFUNCTION()
	virtual void OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
public:
	//TODO: 프로젝트용 구조체로 변경하기 _ 이건주 _ 손승우
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponData")
	// FMeleeWeaponData WeaponData;
	
	// USTRUCT(BlueprintType)
	// struct FHeroAbilitySet
	// {
	// 	GENERATED_BODY()
	//
	// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="InputTag"))
	// 	FGameplayTag InputTag;
	//
	// 	//부여가능한 능력
	// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// 	TSubclassOf<UBaseGameplayAbility> AbilityToGrant;
	//
	// 	bool IsValid() const;
	// };
	//
	// USTRUCT(BlueprintType)
	// struct FHeroWeaponData
	// {
	// 	GENERATED_BODY()
	//
	// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// 	TSubclassOf<UHeroLinkedAnimLayer> WeaponAnimLayerToLink;
	//
	// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// 	UInputMappingContext* WeaponInputMappingContext;
	//
	// 	//무기능력
	// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(TitleProperty = "InputTag"))
	// 	TArray<FHeroAbilitySet> WeaponAbilities;
	// };
};
