// Fill out your copyright notice in the Description page of Project Settings.
// 제작자 : 이건주
// 제작일 : 2026-03-10
// 수정자 : 이건주
// 수정일 : 2026-03-10
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "MHAttributeSetMacro.h"
#include "MHResistanceAttributeSet.generated.h"

UCLASS()
class PROJECTMHW_API UMHResistanceAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMHResistanceAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Resistance", ReplicatedUsing=OnRep_FireResist)
	FGameplayAttributeData FireResist;
	ATTRIBUTE_ACCESSORS(UMHResistanceAttributeSet, FireResist)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Resistance", ReplicatedUsing=OnRep_WaterResist)
	FGameplayAttributeData WaterResist;
	ATTRIBUTE_ACCESSORS(UMHResistanceAttributeSet, WaterResist)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Resistance", ReplicatedUsing=OnRep_ThunderResist)
	FGameplayAttributeData ThunderResist;
	ATTRIBUTE_ACCESSORS(UMHResistanceAttributeSet, ThunderResist)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Resistance", ReplicatedUsing=OnRep_IceResist)
	FGameplayAttributeData IceResist;
	ATTRIBUTE_ACCESSORS(UMHResistanceAttributeSet, IceResist)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Resistance", ReplicatedUsing=OnRep_DragonResist)
	FGameplayAttributeData DragonResist;
	ATTRIBUTE_ACCESSORS(UMHResistanceAttributeSet, DragonResist)

protected:
	UFUNCTION()
	void OnRep_FireResist(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_WaterResist(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ThunderResist(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_IceResist(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DragonResist(const FGameplayAttributeData& OldValue);
};