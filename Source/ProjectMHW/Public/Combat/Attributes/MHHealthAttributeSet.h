// Fill out your copyright notice in the Description page of Project Settings.
// 제작자 : 이건주
// 제작일 : 2026-03-10
// 수정자 : 이건주
// 수정일 : 2026-03-10
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "MHAttributeSetMacro.h"
#include "MHHealthAttributeSet.generated.h"

UCLASS()
class PROJECTMHW_API UMHHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UMHHealthAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:

	/* 현재 체력 */

	UPROPERTY(BlueprintReadOnly, Category="Health", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMHHealthAttributeSet, Health)

	/* 최대 체력 */

	UPROPERTY(BlueprintReadOnly, Category="Health", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMHHealthAttributeSet, MaxHealth)

	/* ExecutionCalculation에서 계산된 대미지 */

	UPROPERTY(BlueprintReadOnly, Category="Health")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UMHHealthAttributeSet, IncomingDamage)

	/* 회복량 */

	UPROPERTY(BlueprintReadOnly, Category="Health")
	FGameplayAttributeData IncomingHeal;
	ATTRIBUTE_ACCESSORS(UMHHealthAttributeSet, IncomingHeal)

protected:

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

};