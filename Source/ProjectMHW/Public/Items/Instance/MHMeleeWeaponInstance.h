// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "MHWeaponInstance.h"
#include "MHMeleeWeaponInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHMeleeWeaponInstance, Log, All);

class AMHPlayerCharacter;
class UBoxComponent;
struct FMHHitAcknowledge;

struct FMHHelmbreakerDelayedHitContext
{
	TWeakObjectPtr<AActor> TargetActor;
	FGameplayEffectSpecHandle DamageSpecHandle;
	FGameplayTag AttackTag;
	FTimerHandle TimerHandle;
	int32 RemainingHitCount = 0;
	int32 AttackSequenceId = INDEX_NONE;
};

/**
 * 근접 무기 인스턴스
 * - 현재 공격의 DamageSpec / AttackTag를 보관한다.
 * - 공격 판정 박스의 오버랩을 감지한다.
 * - 감지된 대상에게 DamageSpec을 전달한다.
 */
UCLASS()
class PROJECTMHW_API AMHMeleeWeaponInstance : public AMHWeaponInstance
{
	GENERATED_BODY()

public:
	AMHMeleeWeaponInstance();

public:
	/** 공격 판정 윈도우를 시작한다. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void BeginAttackWindow();

	/** 공격 판정 윈도우를 종료한다. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void EndAttackWindow();

	/** 현재 근접 공격 상태를 초기화한다. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ResetMeleeAttack();

	/** 히트 박스 콜리전을 켜고 끈다. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetAttackCollisionEnabled(bool bEnabled);

	/** 현재 윈도우에서 이미 맞은 대상을 초기화한다. */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ClearHitActors();

	/** 현재 공격의 DamageSpec을 설정한다. */
	void SetCurrentDamageSpec(const FGameplayEffectSpecHandle& InDamageSpecHandle);

	/** 현재 공격 태그를 설정한다. */
	void SetCurrentAttackTag(const FGameplayTag& InAttackTag);

	/** 현재 공격 데이터를 비운다. */
	void ClearCurrentAttackData();

	/** 현재 DamageSpec이 유효한지 확인한다. */
	bool HasValidCurrentDamageSpec() const;

	/** 현재 공격 태그를 반환한다. */
	const FGameplayTag& GetCurrentAttackTag() const { return CurrentAttackTag; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 유효한 히트가 확정됐을 때 카메라 셰이크를 재생한다. */
	void PlayAcceptedHitCameraShake(const FGameplayTag& InAttackTag);

	/** 현재 공격 데이터 기준으로 DamageSpec을 전달한다. */
	bool TryDeliverDamageSpecToTarget(
		AActor* TargetActor,
		const FHitResult& HitResult,
		FMHHitAcknowledge& OutHitAcknowledge
	);

	/** 지정한 공격 데이터 기준으로 DamageSpec을 전달한다. */
	bool TryDeliverDamageSpecToTargetWithAttackData(
		AActor* TargetActor,
		const FHitResult& HitResult,
		const FGameplayTag& InAttackTag,
		const FGameplayEffectSpecHandle& InDamageSpecHandle,
		FMHHitAcknowledge& OutHitAcknowledge
	);

	/** 오버랩 정보 기준으로 실제 히트 결과를 구성한다. */
	FHitResult BuildResolvedHitResult(
		UPrimitiveComponent* OtherComp,
		AActor* OtherActor,
		bool bFromSweep,
		const FHitResult& SweepResult
	) const;

	/** 실제 타격 지점을 계산한다. */
	FVector ResolveImpactPoint(
		UPrimitiveComponent* OtherComp,
		AActor* OtherActor,
		bool bFromSweep,
		const FHitResult& SweepResult
	) const;

	/** 공격 윈도우 시작 시 이미 겹친 대상을 다시 검사한다. */
	void ProcessExistingOverlapsAtAttackWindowBegin();

	/** 새 공격 데이터가 들어왔을 때 1회성 상태를 초기화한다. */
	void ResetPerAttackRuntimeState();

	/** 투구깨기 지연 다단히트 전용 처리가 필요한지 확인한다. */
	bool ShouldUseHelmbreakerDelayedMultiHit() const;

	/** 투구깨기 첫 오버랩 대상을 지연 5연타 대상으로 등록한다. */
	bool ScheduleHelmbreakerDelayedMultiHit(AActor* InTargetActor);

	/** 등록된 투구깨기 지연 타격을 1회 실행한다. */
	void ExecuteHelmbreakerDelayedMultiHit(TWeakObjectPtr<AActor> InTargetActor);

	/** 등록된 투구깨기 지연 타격 컨텍스트를 찾는다. */
	FMHHelmbreakerDelayedHitContext* FindHelmbreakerDelayedHitContext(AActor* InTargetActor);

	/** 등록된 투구깨기 지연 타격 컨텍스트를 제거한다. */
	void RemoveHelmbreakerDelayedHitContext(AActor* InTargetActor);

	/** 등록된 투구깨기 지연 타이머를 모두 정리한다. */
	void ClearHelmbreakerDelayedHitContexts();

	/** 공격별 최초 확정 히트 보상을 1회만 처리한다. */
	void ResolveConfirmedHitForAttack(
		AMHPlayerCharacter* PlayerOwner,
		const FGameplayTag& InAttackTag,
		int32 InAttackSequenceId,
		const FMHHitAcknowledge& HitAcknowledge
	);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UBoxComponent> HitBox;

	/** 현재 공격 윈도우에서 이미 맞은 대상 목록 */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> HitActors;

	/** 현재 공격 DamageSpec */
	UPROPERTY(Transient)
	FGameplayEffectSpecHandle CurrentDamageSpecHandle;

	/** 현재 공격 태그 */
	UPROPERTY(Transient)
	FGameplayTag CurrentAttackTag;

	/** 현재 공격에서 확정 히트 보상이 이미 처리됐는지 기록 */
	UPROPERTY(Transient)
	bool bResolvedConfirmedHitForCurrentAttack = false;

	/** 현재 공격 데이터에 해당하는 순번 */
	UPROPERTY(Transient)
	int32 CurrentAttackSequenceId = INDEX_NONE;

	/** 최초 확정 히트를 이미 처리한 공격 순번 목록 */
	UPROPERTY(Transient)
	TSet<int32> ResolvedConfirmedHitAttackSequenceIds;

	/** 투구깨기 지연 다단히트 예약 목록 */
	TArray<FMHHelmbreakerDelayedHitContext> HelmbreakerDelayedHitContexts;

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
