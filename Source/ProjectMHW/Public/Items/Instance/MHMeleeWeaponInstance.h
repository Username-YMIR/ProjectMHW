// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "MHWeaponInstance.h"
#include "MHMeleeWeaponInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHMeleeWeaponInstance, Log, All);

class UBoxComponent;
struct FMHHitAcknowledge;

/**
 * 근접 무기 인스턴스
 * - GA가 생성한 현재 공격용 DamageSpec / AttackTag를 보관
 * - 히트 윈도우 동안 오버랩을 감지
 * - 피격자 인터페이스에 DamageSpec을 전달
 */
UCLASS()
class PROJECTMHW_API AMHMeleeWeaponInstance : public AMHWeaponInstance
{
	GENERATED_BODY()

public:
	AMHMeleeWeaponInstance();

public:
	/** 공격 시작 혹은 끝에서 호출 */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ResetMeleeAttack();

	/** 히트 콜리전 On/Off */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetAttackCollisionEnabled(bool bEnabled);

	/** 이번 공격 윈도우에서 맞은 대상 초기화 */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ClearHitActors();

	/** 현재 공격용 DamageSpec 저장 */
	void SetCurrentDamageSpec(const FGameplayEffectSpecHandle& InDamageSpecHandle);

	/** 현재 공격 식별용 태그 저장 */
	void SetCurrentAttackTag(const FGameplayTag& InAttackTag);

	/** 현재 공격 데이터 초기화 */
	void ClearCurrentAttackData();

	/** 현재 DamageSpec이 유효한지 확인 */
	bool HasValidCurrentDamageSpec() const;

	/** 현재 공격 태그 조회 */
	const FGameplayTag& GetCurrentAttackTag() const { return CurrentAttackTag; }

protected:
	virtual void BeginPlay() override;

	/** 피격자에게 DamageSpec 전달 */
	bool TryDeliverDamageSpecToTarget(
		AActor* TargetActor,
		const FHitResult& HitResult,
		FMHHitAcknowledge& OutHitAcknowledge
	);
	
protected:
	FHitResult BuildResolvedHitResult(
		UPrimitiveComponent* OtherComp,
		AActor* OtherActor,
		bool bFromSweep,
		const FHitResult& SweepResult
	) const;

	FVector ResolveImpactPoint(
		UPrimitiveComponent* OtherComp,
		AActor* OtherActor,
		bool bFromSweep,
		const FHitResult& SweepResult
	) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UBoxComponent> HitBox;

	/** 이번 공격 판정 중 이미 맞은 액터 목록 */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> HitActors;

	/** GA가 생성해서 전달한 현재 공격용 DamageSpec */
	UPROPERTY(Transient)
	FGameplayEffectSpecHandle CurrentDamageSpecHandle;

	/** 현재 기술 식별용 태그 */
	UPROPERTY(Transient)
	FGameplayTag CurrentAttackTag;

	/** 이번 공격에서 자원 반영을 이미 처리했는지 기록 */
	UPROPERTY(Transient)
	bool bResolvedConfirmedHitForCurrentAttack = false;

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
};