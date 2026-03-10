// Fill out your copyright notice in the Description page of Project Settings.
// 제작자 : 이건주
// 제작일 : 2026-03-10
// 수정자 : 이건주
// 수정일 : 2026-03-10
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "MHAttributeSetMacro.h"
#include "MHCombatAttributeSet.generated.h"

UCLASS()
class PROJECTMHW_API UMHCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMHCombatAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

public:
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Combat", ReplicatedUsing=OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UMHCombatAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Combat", ReplicatedUsing=OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UMHCombatAttributeSet, Defense)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Combat", ReplicatedUsing=OnRep_CriticalRate)
	FGameplayAttributeData CriticalRate;
	ATTRIBUTE_ACCESSORS(UMHCombatAttributeSet, CriticalRate)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Combat", ReplicatedUsing=OnRep_SharpnessModifier)
	FGameplayAttributeData SharpnessModifier;
	ATTRIBUTE_ACCESSORS(UMHCombatAttributeSet, SharpnessModifier)

protected:
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_SharpnessModifier(const FGameplayAttributeData& OldValue);
};