// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHWeaponInstance.h"
#include "MHMeleeWeaponInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHMeleeWeaponInstance, Log, All)


class UBoxComponent;
class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class PROJECTMHW_API AMHMeleeWeaponInstance : public AMHWeaponInstance
{
	GENERATED_BODY()

public:
	AMHMeleeWeaponInstance();

protected:
	virtual void BeginPlay() override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UBoxComponent> HitBox;

	/** 이번 공격 판정 중 이미 맞은 액터 목록 */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> DamagedActors;

	/** 대미지 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 기본 물리 대미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float BasePhysicalDamage = 30.f;

	/** 기본 화속성 대미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float BaseFireDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float BaseWaterDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float BaseThunderDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float BaseIceDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Damage")
	float BaseDragonDamage = 0.f;

protected:
	UFUNCTION()
	void OnWeaponBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	/** 공격 시작 혹은 끝에서 호출 */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ClearDamagedActors();
};