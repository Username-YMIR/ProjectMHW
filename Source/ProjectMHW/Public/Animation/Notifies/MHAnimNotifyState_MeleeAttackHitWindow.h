// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MHAnimNotifyState_MeleeAttackHitWindow.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(MHANS_MeleeAttackHitWindow, Log, All);


class AMHMeleeWeaponInstance;
class USkeletalMeshComponent;

/**
 * 근접 무기의 공격 판정 유효 구간을 제어하는 AnimNotifyState
 * - 시작 시 공격 콜리전 활성화
 * - 종료 시 공격 콜리전 비활성화
 */
UCLASS(meta = (DisplayName = "MH Melee Attack Hit Window"))
class PROJECTMHW_API UMHAnimNotifyState_MeleeAttackHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

private:
	/** Notify가 붙은 메시의 소유 액터를 가져온다. */
	AActor* ResolveOwnerActor(USkeletalMeshComponent* MeshComp) const;

	/** 소유 액터가 장착 중인 근접 무기를 찾아 반환한다. */
	AMHMeleeWeaponInstance* ResolveMeleeWeapon(AActor* OwnerActor) const;
};