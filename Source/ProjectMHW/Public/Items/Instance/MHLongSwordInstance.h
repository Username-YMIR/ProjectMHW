// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHMeleeWeaponInstance.h"
#include "MHLongSwordInstance.generated.h"

class UMHLongSwordComboGraph; // 손승우 추가
class UMHLongSwordComboComponent; // 손승우 추가

UCLASS()
class PROJECTMHW_API AMHLongSwordInstance : public AMHMeleeWeaponInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHLongSwordInstance();

protected:
	virtual void BeginPlay() override; // 손승우 추가
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* SayaMesh; // 손승우 수정

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combo", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UMHLongSwordComboGraph> ComboGraphAsset; // 손승우 추가

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Combo", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHLongSwordComboComponent> ComboComponent; // 손승우 추가

public:
	virtual USkeletalMeshComponent* GetSheathMeshComponent() const override { return SayaMesh; } // 손승우 추가

	// 태도 콤보 컴포넌트
	FORCEINLINE UMHLongSwordComboComponent* GetComboComponent() const { return ComboComponent; } // 손승우 추가

	// 태도 콤보 그래프
	UMHLongSwordComboGraph* GetComboGraph() const; // 손승우 추가
};
