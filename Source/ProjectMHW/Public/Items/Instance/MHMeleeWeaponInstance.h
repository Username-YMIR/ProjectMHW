// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
DECLARE_LOG_CATEGORY_EXTERN(LogMeleeWeaponInstance, Log, All)

#include "CoreMinimal.h"
#include "MHWeaponInstance.h"
#include "MHMeleeWeaponInstance.generated.h"

DECLARE_DELEGATE_OneParam(FOnTargetInteractedDelegate, AActor*)

class UBoxComponent;
struct FMHDamageContext;

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
	// 근접무기 히트 판정 콜리전 박스 _이건주
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UBoxComponent* WeaponCollisionBox;
	
	// 이번 콤보에서 이미 맞은 액터 목록_이건주
	UPROPERTY()
	TArray<TObjectPtr<AActor>> OverlappedActorsThisCombo;

	// 소유자 캐릭터 또는 공격자_이건주
	UPROPERTY()
	TWeakObjectPtr<AActor> CachedOwnerActor;

	// 테스트용 값_이건주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	float TestBaseDamage = 10.f;

	// 테스트용 콤보 번호_이건주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	int32 CurrentComboIndex = 0;
	
public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const{return WeaponCollisionBox;}

	UFUNCTION()
	virtual void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
   
	UFUNCTION()
	virtual void OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	FMHDamageContext BuildDamageContext(AActor* TargetActor, const FHitResult& HitResult) const;
	
};
